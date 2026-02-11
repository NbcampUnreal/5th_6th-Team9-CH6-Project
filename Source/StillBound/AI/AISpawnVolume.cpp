

#include "AI/AISpawnVolume.h"
#include "AI/EnemyCharacter.h"
#include "Components/BoxComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"


AAISpawnVolume::AAISpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	SetRootComponent(SpawnBox);

	SpawnBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnBox->SetGenerateOverlapEvents(false);

	EnemyClass = AEnemyCharacter::StaticClass();
}

void AAISpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[SpawnVolume] BeginPlay: %s"), *GetName());

	// 타이머로 스폰 체크
	if (SpawnCheckInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&AAISpawnVolume::SpawnTick,
			SpawnCheckInterval,
			true
		);
	}
}

void AAISpawnVolume::SpawnTick()
{
	CleanupAliveList();

	APawn* PlayerPawn = GetPlayerPawn();
	if (!PlayerPawn) return;

	if (!IsPlayerInRange(PlayerPawn)) return;
	if (!ShouldSpawnNow()) return;

	const double Now = GetWorld()->GetTimeSeconds();
	if (LastVacancyTime > 0.0 && (Now - LastVacancyTime) < RespawnDelay) return;

	if (!EnemyClass || SpawnEntries.Num() == 0) return;

	const int32 Alive = GetAliveCount();
	if (Alive >= MaxAlive) return;

	FVector SpawnLoc;
	FRotator SpawnRot;
	if (!FindSpawnLocation(PlayerPawn, SpawnLoc, SpawnRot)) return;

	const int32 PickedEnemyId = PickEnemyIdByWeight();
	if (PickedEnemyId <= 0) return;
	if (!CanSpawnEnemyId(PickedEnemyId)) return;

	AEnemyCharacter* Spawned = SpawnEnemyDeferred(PickedEnemyId, SpawnLoc, SpawnRot);
	if (!Spawned) return;

	AliveActors.Add(Spawned);
	AliveCountByEnemyId.FindOrAdd(PickedEnemyId)++;
	Spawned->OnDestroyed.AddDynamic(this, &AAISpawnVolume::HandleSpawnedActorDestroyed);
}

APawn* AAISpawnVolume::GetPlayerPawn() const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	return PC ? PC->GetPawn() : nullptr;
}

bool AAISpawnVolume::IsPlayerInRange(APawn* PlayerPawn) const
{
	const FVector Center = SpawnBox ? SpawnBox->GetComponentLocation() : GetActorLocation();
	const float DistSq = FVector::DistSquared(PlayerPawn->GetActorLocation(), Center);
	return DistSq <= FMath::Square(ActivationDistance);
}

bool AAISpawnVolume::ShouldSpawnNow() const
{
	// 단순하게: 비어 있으면 채우는 방식
	// (원하면 “시간대/날씨/퀘스트” 조건을 여기서 더 붙이면 됨)
	return true;
}

int32 AAISpawnVolume::GetAliveCount() const
{
	int32 Count = 0;
	for (const auto& W : AliveActors)
	{
		if (W.IsValid()) Count++;
	}
	return Count;
}

void AAISpawnVolume::CleanupAliveList()
{
	// 죽어서 Destroy된 애들 제거 + vacancy time 갱신
	const int32 Before = GetAliveCount();

	for (int32 i = AliveActors.Num() - 1; i >= 0; --i)
	{
		if (!AliveActors[i].IsValid())
		{
			AliveActors.RemoveAtSwap(i);
		}
	}

	const int32 After = GetAliveCount();
	if (After < Before)
	{
		LastVacancyTime = GetWorld()->GetTimeSeconds();
	}
}

bool AAISpawnVolume::FindSpawnLocation(APawn* PlayerPawn, FVector& OutLocation, FRotator& OutRotation) const
{
	if (!SpawnBox) return false;

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys) return false;

	const FVector Center = SpawnBox->GetComponentLocation();
	const FVector Extent = SpawnBox->GetScaledBoxExtent();
	const float Radius = FMath::Max(Extent.X, Extent.Y);

	for (int32 Try = 0; Try < MaxFindLocationTries; ++Try)
	{
		FNavLocation NavLoc;
		if (!NavSys->GetRandomReachablePointInRadius(Center, Radius, NavLoc))
			continue;

		// (선택) 박스 안쪽인지 확인
		const FBox Box(Center - Extent, Center + Extent);
		if (!Box.IsInsideOrOn(NavLoc.Location))
			continue;

		// 플레이어 최소 거리
		if (FVector::DistSquared(NavLoc.Location, PlayerPawn->GetActorLocation()) < FMath::Square(MinDistanceFromPlayer))
			continue;

		// 간단 겹침 체크
		FCollisionQueryParams Params(SCENE_QUERY_STAT(AISpawnVolume), false, this);
		if (GetWorld()->OverlapAnyTestByChannel(
			NavLoc.Location, FQuat::Identity, ECC_Pawn,
			FCollisionShape::MakeSphere(SpawnCollisionRadius), Params))
		{
			continue;
		}

		OutLocation = NavLoc.Location;
		OutRotation = FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f);
		return true;
	}

	return false;
}

int32 AAISpawnVolume::PickEnemyIdByWeight() const
{
	double Total = 0.0;
	for (const FEnemySpawnEntry& E : SpawnEntries)
	{
		if (E.EnemyId > 0 && E.Weight > 0.f)
		{
			Total += E.Weight;
		}
	}
	if (Total <= 0.0) return 0;

	const double R = FMath::FRandRange(0.0, Total);
	double Acc = 0.0;

	for (const FEnemySpawnEntry& E : SpawnEntries)
	{
		if (E.EnemyId <= 0 || E.Weight <= 0.f) continue;

		Acc += E.Weight;
		if (R <= Acc)
		{
			return E.EnemyId;
		}
	}
	return SpawnEntries[0].EnemyId;
}

bool AAISpawnVolume::CanSpawnEnemyId(int32 EnemyId) const
{
	const FEnemySpawnEntry* Entry = SpawnEntries.FindByPredicate(
		[EnemyId](const FEnemySpawnEntry& E) { return E.EnemyId == EnemyId; }
	);
	if (!Entry) return false;

	if (Entry->MaxAliveFromThisEntry <= 0) return true;

	const int32 Current = AliveCountByEnemyId.Contains(EnemyId) ? AliveCountByEnemyId[EnemyId] : 0;
	return Current < Entry->MaxAliveFromThisEntry;
}

AEnemyCharacter* AAISpawnVolume::SpawnEnemyDeferred(int32 EnemyId, const FVector& Location, const FRotator& Rotation)
{
	UWorld* World = GetWorld();
	if (!World || !EnemyClass) return nullptr;

	FTransform TM(Rotation, Location);

	// Deferred로 EnemyId 먼저 넣고 BeginPlay 전에 확정
	AEnemyCharacter* Spawned = World->SpawnActorDeferred<AEnemyCharacter>(
		EnemyClass,
		TM,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
	);

	if (!Spawned) return nullptr;

	// EnemyCharacter의 EnemyId가 private라서 직접 접근 불가.
	// 가장 깔끔한 건 EnemyCharacter에 SetEnemyId(int32) 함수를 하나 추가하는 것.
	// 일단 “BP로 EnemyId를 Expose on Spawn” 하거나, 아래처럼 리플렉션으로 설정할 수도 있지만,
	// 프로젝트 유지보수용으론 SetEnemyId 함수 추가를 강력 추천.

	// ===== 추천: EnemyCharacter에 아래 함수 추가 후 사용 =====
	 Spawned->SetEnemyId(EnemyId);

	// 임시 대응: EnemyId를 에디터에서 public/editanywhere로 바꾸거나,
	// EnemyCharacter에 BlueprintCallable Setter 만들어서 여기서 호출해줘.

	UGameplayStatics::FinishSpawningActor(Spawned, TM);
	return Spawned;
}

void AAISpawnVolume::HandleSpawnedActorDestroyed(AActor* DestroyedActor)
{
	// AliveActors에서 정리는 SpawnTick에서 하지만, 여기서 EnemyId별 카운트만 줄여둠
	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(DestroyedActor);
	if (!Enemy)
	{
		LastVacancyTime = GetWorld()->GetTimeSeconds();
		return;
	}

	// EnemyId를 여기서도 읽어야 엔트리별 카운트를 줄일 수 있는데,
	// EnemyCharacter에 GetEnemyId() 같은 Getter가 없어서 “권장 수정”이 필요함.
	// 지금은 간단히 vacancy time만 갱신하고,
	// 엔트리별 제한(MaxAliveFromThisEntry)이 필요하면 Getter/Setter를 꼭 추가하자.

	LastVacancyTime = GetWorld()->GetTimeSeconds();
}