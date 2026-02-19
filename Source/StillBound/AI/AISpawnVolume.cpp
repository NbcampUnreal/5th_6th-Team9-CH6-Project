

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
	if (!PlayerPawn)
	{
		return;
	}

	if (!IsPlayerInRange(PlayerPawn))
	{
		return;
	}
	if (!ShouldSpawnNow())
	{
		return;
	}

	const double Now = GetWorld()->GetTimeSeconds();
	if (LastVacancyTime > 0.0 && (Now - LastVacancyTime) < RespawnDelay)
	{
		return;
	}

	if (!EnemyClass || SpawnEntries.Num() == 0)
	{
		return;
	}

	const int32 Alive = GetAliveCount();
	if (Alive >= MaxAlive)
	{
		return;
	}

	FVector SpawnLoc;
	FRotator SpawnRot;
	if (!FindSpawnLocation(PlayerPawn, SpawnLoc, SpawnRot))
	{
		return;
	}

	const int32 PickedEnemyId = PickEnemyIdByWeight();
	if (PickedEnemyId <= 0)
	{
		return;
	}
	if (!CanSpawnEnemyId(PickedEnemyId))
	{
		return;
	}

	AEnemyCharacter* Spawned = SpawnEnemyDeferred(PickedEnemyId, SpawnLoc, SpawnRot);
	if (!Spawned)
	{
		return;
	}

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
	if (!SpawnBox)
	{
		return false;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		return false;
	}

	const FVector Center = SpawnBox->GetComponentLocation();
	const FVector Extent = SpawnBox->GetScaledBoxExtent();
	const float Radius = FMath::Max(Extent.X, Extent.Y);

	for (int32 Try = 0; Try < MaxFindLocationTries; ++Try)
	{
		FNavLocation NavLoc;
		if (!NavSys->GetRandomReachablePointInRadius(Center, Radius, NavLoc))
		{
			continue;
		}

		const FBox Box(Center - Extent, Center + Extent);
		if (!Box.IsInsideOrOn(NavLoc.Location))
		{
			continue;
		}


		if (FVector::DistSquared(NavLoc.Location, PlayerPawn->GetActorLocation()) < FMath::Square(MinDistanceFromPlayer))
		{
			continue;
		}

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
	if (Total <= 0.0)
	{
		return 0;
	}

	const double R = FMath::FRandRange(0.0, Total);
	double Acc = 0.0;

	for (const FEnemySpawnEntry& E : SpawnEntries)
	{
		if (E.EnemyId <= 0 || E.Weight <= 0.f)
		{
			continue;
		}

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
	if (!Entry)
	{
		return false;
	}

	if (Entry->MaxAliveFromThisEntry <= 0)
	{
		return true;
	}

	const int32 Current = AliveCountByEnemyId.Contains(EnemyId) ? AliveCountByEnemyId[EnemyId] : 0;
	return Current < Entry->MaxAliveFromThisEntry;
}

AEnemyCharacter* AAISpawnVolume::SpawnEnemyDeferred(int32 EnemyId, const FVector& Location, const FRotator& Rotation)
{
	UWorld* World = GetWorld();
	if (!World || !EnemyClass)
	{
		return nullptr;
	}

	FTransform TM(Rotation, Location);

	AEnemyCharacter* Spawned = World->SpawnActorDeferred<AEnemyCharacter>(
		EnemyClass,
		TM,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
	);

	if (!Spawned)
	{
		return nullptr;
	}

	 Spawned->SetEnemyId(EnemyId);

	UGameplayStatics::FinishSpawningActor(Spawned, TM);
	return Spawned;
}

void AAISpawnVolume::HandleSpawnedActorDestroyed(AActor* DestroyedActor)
{
	for (int32 i = AliveActors.Num() - 1; i >= 0; --i)
	{
		if (!AliveActors[i].IsValid() || AliveActors[i].Get() == DestroyedActor)
		{
			AliveActors.RemoveAtSwap(i);
		}
	}

	if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(DestroyedActor))
	{
		const int32 EnemyId = Enemy->GetEnemyId();

		if (int32* Cnt = AliveCountByEnemyId.Find(EnemyId))
		{
			*Cnt = FMath::Max(0, *Cnt - 1);
			if (*Cnt == 0)
			{
				AliveCountByEnemyId.Remove(EnemyId);
			}
		}
	}

	LastVacancyTime = GetWorld()->GetTimeSeconds();
}