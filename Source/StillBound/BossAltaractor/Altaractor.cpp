//Altaractor.cpp

#include "BossAltaractor/Altaractor.h"
#include "BossAltaractor/BossArenawall.h"
#include "AI/EnemyCharacter.h"
#include "Character/PlayerCharacter_SB.h"
#include "Character/PlayerController_SB.h"
#include "Inventory/InventoryComponent.h"
#include "UI/USB_UIManager.h"
#include "BossAltaractor/WbpAltarUI.h"
#include "Blueprint/UserWidget.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AAltaractor::AAltaractor()
{
	PrimaryActorTick.bCanEverTick = false;

	AltarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AltarMesh"));
	RootComponent = AltarMesh;

	//기본 상호작용 데이터 세팅
	InteractableData.InteractableType = EInteractableType::Device;
	InteractableData.Name = FText::FromString(TEXT("Ancient Altar"));
	InteractableData.Action = FText::FromString(TEXT("Unseal"));
	InteractableData.InteractionDuration = 0.f;
}

void AAltaractor::BeginPlay()
{
	Super::BeginPlay();

	// 리스폰용 위치 저장
	AltarSpawnLocation = GetActorLocation();
	AltarSpawnRotation = GetActorRotation();
	AltarSpawnScale = GetActorScale3D();
	// 실제 BP 클래스 저장 ,레벨에 배치된 인스턴스의 클래스
	AltarBPClass = GetClass();
}

//===상호작용 인터페이스
void AAltaractor::BeginFocus_Implementation()
{
	if (AltarMesh)
	{
		AltarMesh->SetRenderCustomDepth(true);
	}
}

void AAltaractor::EndFocus_Implementation()
{
	if (AltarMesh)
	{
		AltarMesh->SetRenderCustomDepth(false);
	}
}

void AAltaractor::BeginInteract_Implementation()
{
}

void AAltaractor::EndInteract_Implementation()
{
}

void AAltaractor::Interact_Implementation(APlayerCharacter_SB* PlayerCharacter)
{
	if (bActivated || !PlayerCharacter) return;
	if (!AltarUIClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Altar] AltarUIClass is not set!"));
		return;
	}

	InteractingPlayer = PlayerCharacter;

	// 이미 열려있으면 닫기 토글
	if (AltarUIInstance && AltarUIInstance->IsInViewport())
	{
		CloseAltarUI();
		return;
	}

	APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());
	if (!PC) return;

	AltarUIInstance = CreateWidget<UUserWidget>(PC, AltarUIClass);
	if (!AltarUIInstance) return;

	AltarUIInstance->AddToViewport(10);
	//키보드 포커스 설정
	AltarUIInstance->SetUserFocus(PC);
	AltarUIInstance->SetKeyboardFocus();

	// UI에 제단 레퍼런스 전달 (WBP_AltarUI는 AAltarActor* 를 받는 함수를 구현해야 함)
	//if (AltarUIInstance->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
	//{
	//	// 인터페이스 없이 직접 캐스팅 방식을 사용 (WBP_AltarUI.h 참고)
	//}

	// BlueprintImplementableEvent로 노출된 함수 호출
	UFunction* InitFunc = AltarUIInstance->FindFunction(FName("InitWithAltar"));
	if (InitFunc)
	{
		struct { AAltaractor* Altar; } Params;
		Params.Altar = this;
		AltarUIInstance->ProcessEvent(InitFunc, &Params);
	}

	// 마우스 커서 활성화
	PC->SetShowMouseCursor(true);
	//PC->SetInputMode(FInputModeGameAndUI());

	// UI Only 모드 - 마우스만 작동, WASD 차단
	FInputModeUIOnly UIOnlyMode;
	UIOnlyMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	UIOnlyMode.SetWidgetToFocus(AltarUIInstance->TakeWidget());
	PC->SetInputMode(UIOnlyMode);

	// 캐릭터 이동 완전 차단
	if (UCharacterMovementComponent* MoveComp =
		InteractingPlayer->GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}
}

FInteractableData AAltaractor::GetInteractableData_Implementation()
{
	return FInteractableData();
}

float AAltaractor::GetInteractionDistance_Implementation()
{
	return 250.f;
}

//UI에서 석판 넣기 버튼 클릭 시 호출. 인벤토리에서 1개 소비 해서 슬롯 채움.
bool AAltaractor::TryInsertTablet()
{
	if (bActivated) return false;
	if (FilledSlotCount >= RequiredSlotCount) return false;

	if (!InteractingPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Altar] No interacting player"));
		return false;
	}

	UInventoryComponent* Inv = InteractingPlayer->GetInventory();
	if (!Inv) return false;

	// 인벤토리에 석판 있는지 확인 후 1개 소비
	const int32 HasCount = Inv->GetTotalCountByID_ForUI(RequiredItemID);
	if (HasCount <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[Altar] Player has no %s"), *RequiredItemID.ToString());
		return false;
	}

	const bool bConsumed = Inv->ConsumeByID(RequiredItemID, 1);
	if (!bConsumed)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Altar] ConsumeByID failed for %s"), *RequiredItemID.ToString());
		return false;
	}

	++FilledSlotCount;
	UE_LOG(LogTemp, Log, TEXT("[Altar] Tablet inserted. %d / %d"), FilledSlotCount, RequiredSlotCount);

	// UI에 갱신 알림 추가
	if (AltarUIInstance)
	{
		UWbpAltarUI* AltarUI = Cast<UWbpAltarUI>(AltarUIInstance);
		if (AltarUI)
		{
			AltarUI->RefreshSlots();
		}
	}

	// 3개 다 채웠으면 소환
	if (FilledSlotCount >= RequiredSlotCount)
	{
		CloseAltarUI();
		ActivateAltar();
	}

	return true;
}

//플레이어가 멀어지거나 ESC키로 UI닫기
void AAltaractor::CloseAltarUI()
{
	if (AltarUIInstance && AltarUIInstance->IsInViewport())
	{
		AltarUIInstance->RemoveFromParent();
	}
	AltarUIInstance = nullptr;

	if (InteractingPlayer)
	{
		// 이동 복원
		if (UCharacterMovementComponent* MoveComp =
			InteractingPlayer->GetCharacterMovement())
		{
			MoveComp->SetMovementMode(EMovementMode::MOVE_Walking);
		}

		APlayerController* PC = Cast<APlayerController>(
			InteractingPlayer->GetController());
		if (PC)
		{
			PC->SetShowMouseCursor(false);
			// 게임 입력 모드 복원
			FInputModeGameOnly GameOnlyMode;
			PC->SetInputMode(GameOnlyMode);
		}
	}
}

//3개 다 채웟을 때 보스 소환, 벽 생성, 제단 소멸
void AAltaractor::ActivateAltar()
{
	if (bActivated) return;
	bActivated = true;

	UWorld* World = GetWorld();
	if (!World) return;

	// 보스 스폰
	if (BossClass)
	{
		const FVector SpawnLoc = GetActorLocation() + BossSpawnOffset;
		const FRotator SpawnRot = GetActorRotation();

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ACharacter* Boss = World->SpawnActor<ACharacter>(BossClass, SpawnLoc, SpawnRot, Params);
		if (Boss)
		{
			SpawnedBoss = Boss;

			// 보스 코드 수정 없이 외부에서 사망 감지
			Boss->OnDestroyed.AddDynamic(this, &AAltaractor::OnBossDestroyed);

			UE_LOG(LogTemp, Log, TEXT("[Altar] Boss spawned: %s"), *GetNameSafe(Boss));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Altar] Boss spawn failed!"));
		}
	}

	//아레나 블로킹 벽 생성
	SpawnArenaWalls();

	//제단 파괴
	//Destroy();
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
}

//아레나 블로킹 벽 배치
void AAltaractor::SpawnArenaWalls()
{
	if (!ArenaWallClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FVector Center = GetActorLocation();
	const float Half = ArenaRadius;
	const float WallLength = Half * 2.f;
	const float WallThick = 100.f; // 두께의 절반

	// 4면 정의, 위치 오프셋, 회전, 벽 길이
	struct FWallInfo
	{
		FVector Offset;
		FRotator Rot;
	};

	TArray<FWallInfo> Walls =
	{
		// +X 면 (앞)
		{ FVector(Half + WallThick, 0.f, 0.f), FRotator(0.f,   0.f, 0.f) },
		// -X 면 (뒤)
		{ FVector(-Half - WallThick, 0.f, 0.f), FRotator(0.f, 180.f, 0.f) },
		// +Y 면 (오른쪽)
		{ FVector(0.f,  Half + WallThick, 0.f), FRotator(0.f,  90.f, 0.f) },
		// -Y 면 (왼쪽)
		{ FVector(0.f, -Half - WallThick, 0.f), FRotator(0.f, 270.f, 0.f) }
	};

	for (const FWallInfo& Info : Walls)
    {
		FVector SpawnLoc = Center + Info.Offset;

		// 지면 아래 500까지 내려서 경사면 대응
		SpawnLoc.Z = Center.Z - 500.f + (ArenaWallHeight * 0.5f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ABossArenaWall* Wall = World->SpawnActor<ABossArenaWall>(
			ArenaWallClass, SpawnLoc, Info.Rot, Params);

		if (Wall)
		{
			Wall->SetWallSize(WallLength + WallThick * 4.f, ArenaWallHeight);
			SpawnedWalls.Add(Wall); 
		}
    }

    UE_LOG(LogTemp, Log, TEXT("[Altar] 4 walls spawned. HalfSize=%.0f Height=%.0f"),
        Half, ArenaWallHeight);
}

void AAltaractor::OnBossDestroyed(AActor* DestroyedActor)
{
	UE_LOG(LogTemp, Log, TEXT("[Altar] Boss destroyed, ending arena"));
	EndArena();
}

void AAltaractor::EndArena()
{
	// 아레나 벽 전부 제거
	for (ABossArenaWall* Wall : SpawnedWalls)
	{
		if (IsValid(Wall)) Wall->Destroy();
	}
	SpawnedWalls.Empty();

	UWorld* World = GetWorld();
	TSubclassOf<AAltaractor> ClassToSpawn = AltarBPClass;
	FVector  SpawnLoc = AltarSpawnLocation;
	FRotator SpawnRot = AltarSpawnRotation;
	FVector  SpawnScale = AltarSpawnScale;

	if (!World || !ClassToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Altar] EndArena: World or Class invalid"));
		Destroy();
		return;
	}

	Destroy();

	// FTransform으로 스케일까지 포함해서 스폰
	FTransform SpawnTransform(SpawnRot, SpawnLoc, SpawnScale);

	AAltaractor* NewAltar = World->SpawnActorDeferred<AAltaractor>(
		ClassToSpawn, SpawnTransform);

	if (NewAltar)
	{
		// BeginPlay 전에 스케일 적용
		NewAltar->SetActorScale3D(SpawnScale);

		// BeginPlay 호출 완료
		NewAltar->FinishSpawning(SpawnTransform);

		UE_LOG(LogTemp, Log, TEXT("[Altar] Altar respawned with scale %s"),
			*SpawnScale.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Altar] Altar respawn failed!"));
	}
}



