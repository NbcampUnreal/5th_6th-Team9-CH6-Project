// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NPCCharacter.h"
#include "NPC/NPCAIController.h"
#include "NPC/DialogueComponent.h"
#include "NPC/DialogueWidget.h"
#include "Items/ItemBase.h"
#include "Inventory/InventoryComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ShopWidget.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ANPCCharacter::ANPCCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DialogueComponent = CreateDefaultSubobject<UDialogueComponent>(TEXT("DialogueComponent"));

	AIControllerClass = ANPCAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Pawn,
		ECollisionResponse::ECR_Block);

	// 기본값 설정
	SellableItemTypes = { EItemType::Tool, EItemType::Weapon, EItemType::Ammo, EItemType::Consumable };
	LowTierKeywords = { TEXT("Stone"), TEXT("Wood"), TEXT("Basic") };
	MaxShopItems = 10;
	bOnlyLowTierEquipment = true;

}

// Called when the game starts or when spawned
void ANPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 상점 아이템 초기화
	InitializeShopItems();

	ANPCAIController* AIController = GetNPCAIController();
	if (AIController)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] NPC Character initialized"), *NPCName);
	}

	if (DialogueComponent)
	{
		if (DialogueComponent->MainMenuDataTable)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] MainMenu Table Name: %s"),
				*NPCName, *DialogueComponent->MainMenuDataTable->GetName());
		}
		DialogueComponent->OnDialogueStarted.AddDynamic(this, &ANPCCharacter::OnDialogueStart);
		DialogueComponent->OnDialogueUpdated.AddDynamic(this, &ANPCCharacter::OnDialogueUpdate);
		DialogueComponent->OnDialogueEnded.AddDynamic(this, &ANPCCharacter::OnDialogueEnd);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] DialogueComponent is NULL in BeginPlay!"), *NPCName);
	}
	if (DialogueWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] DialogueWidgetClass: %s"),
			*NPCName, *DialogueWidgetClass->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] DialogueWidgetClass is NULL!"), *NPCName);
	}

	
}

void ANPCCharacter::InitializeShopItems()
{
	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] ItemDataTable is NULL!"), *NPCName);
		return;
	}

	SellableItemIDs.Empty();
	TArray<FName> AllRowNames = ItemDataTable->GetRowNames();

	UE_LOG(LogTemp, Error, TEXT("[%s] Initializing shop from %s (%d items)"),
		*NPCName, *ItemDataTable->GetName(), AllRowNames.Num());

	// 판매할 아이템 타입 (기획서 기준)
	TArray<EItemType> AllowedTypes = {
		EItemType::Tool,       // 도구
		EItemType::Weapon,     // 무기 (Low-tier만)
		EItemType::Consumable,  // 소비 아이템
		EItemType::Ammo
	};

	int32 FilteredCount = 0;

	// 1단계: 조건에 맞는 아이템 필터링
	for (FName RowName : AllRowNames)
	{
		FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(RowName, TEXT("ShopInit"));

		if (!ItemData)
		{
			continue;
		}

		// ItemType 필터
		if (!AllowedTypes.Contains(ItemData->ItemType))
		{
			continue;
		}

		// Low-tier 필터
		if (bOnlyLowTierEquipment &&
			(ItemData->ItemType == EItemType::Weapon || ItemData->ItemType == EItemType::Tool))
		{
			if (!IsLowTierItem_ByRowName(RowName, ItemData->ItemType))
			{
				continue;
			}
		}

		// 통과!
		SellableItemIDs.Add(RowName);
		FilteredCount++;

		UE_LOG(LogTemp, Log, TEXT("  Added: %s"), *RowName.ToString());
	}

	UE_LOG(LogTemp, Error, TEXT("[%s] Shop initialized: %d/%d items"),
		*NPCName, FilteredCount, AllRowNames.Num());
}

bool ANPCCharacter::IsLowTierItem(const FItemDataRow* ItemData) const
{
	if (!ItemData) return false;

	// Consumable, Ammo는 항상 판매
	if (ItemData->ItemType == EItemType::Consumable ||
		ItemData->ItemType == EItemType::Ammo)
	{
		return true;
	}

	// SellValue가 0이면 Row Name으로 판단
	FString RowName = ItemData->ID.ToString();
	if (RowName.IsEmpty())
	{
		// ID가 없으면 모두 Low-tier로 판단
		return true;
	}

	// Row Name 숫자로 판단
	int32 ItemIDNum = FCString::Atoi(*RowName);

	// Tool: 200001~200004 (wooden, stone)
	if (ItemData->ItemType == EItemType::Tool)
	{
		return (ItemIDNum >= 200001 && ItemIDNum <= 200004);
	}

	// Weapon: 110001~110006 (wooden, stone)
	if (ItemData->ItemType == EItemType::Weapon)
	{
		return (ItemIDNum >= 110001 && ItemIDNum <= 110006);
	}

	// 기본적으로 Low-tier
	return true;
}

bool ANPCCharacter::IsLowTierItem_ByRowName(FName RowName, EItemType ItemType) const
{
	FString RowNameStr = RowName.ToString();
	int32 ItemID = FCString::Atoi(*RowNameStr);

	// Tool: 200001~200004 (wooden, stone)
	if (ItemType == EItemType::Tool)
	{
		return (ItemID >= 200001 && ItemID <= 200004);
	}

	// Weapon: 110001~110006 (wooden, stone)
	if (ItemType == EItemType::Weapon)
	{
		return (ItemID >= 110001 && ItemID <= 110006);
	}

	// Consumable, Ammo: 전부 판매
	return true;
}

void ANPCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	MonitorStateChanges();

}

void ANPCCharacter::MonitorStateChanges()
{
	ANPCAIController* AIController = GetNPCAIController();
	if (!AIController) return;

	uint8 CurrentState = static_cast<uint8>(AIController->GetNPCState());

	if (CurrentState != LastNPCState)
	{
		if(CurrentState == static_cast<uint8>(ENPCMode::Alert) &&
			LastNPCState == static_cast<uint8>(ENPCMode::Idle))
		{
			AActor* Target = AIController->GetTargetActor();
			if (Target)
			{
				OnPlayerDetected(Target);
			}
		}
	}
	else if (CurrentState == static_cast<uint8>(ENPCMode::Idle) &&
		LastNPCState == static_cast<uint8>(ENPCMode::Alert))
	{
		OnPlayerLost();
	}
	LastNPCState = CurrentState;
}

void ANPCCharacter::BeginFocus_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[%s] Player looking at me"), *NPCName);
}

void ANPCCharacter::EndFocus_Implementation()
{
	// 플레이어가 시선을 돌렸을 때
	UE_LOG(LogTemp, Log, TEXT("[%s] Player looking away"), *NPCName);
}

void ANPCCharacter::EndInteract_Implementation()
{
	if (DialogueComponent && DialogueComponent->IsDialogueActive())
	{
		DialogueComponent->EndDialogue();
	}
}

void ANPCCharacter::Interact_Implementation(APlayerCharacter_SB* PlayerCharacter)
{
	if (!PlayerCharacter || bIsInteracting) return;

	UE_LOG(LogTemp, Log, TEXT("[%s] Interaction Started with %s"),
		*NPCName, *PlayerCharacter->GetName());

	bIsInteracting = true;
	CurrentInteractor = PlayerCharacter;

	// AI 상태 변경
	if (ANPCAIController* AIController = GetNPCAIController())
	{
		AIController->SetNPCState(ENPCMode::Interacting);
		AIController->SetInteractionTarget(PlayerCharacter);
	}

	// 대화 시작 (DialogueComponent가 이벤트 발동)
	if (DialogueComponent)
	{
		bool bStarted = DialogueComponent->StartDialogue(PlayerCharacter);

		if (bStarted)
		{
			OnInteractionStarted(PlayerCharacter);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to start dialogue"), *NPCName);

			// 즉시 정리
			bIsInteracting = false;
			CurrentInteractor = nullptr;

			if (ANPCAIController* AIController = GetNPCAIController())
			{
				AIController->SetInteractionTarget(nullptr);
				AIController->SetNPCState(ENPCMode::Idle);
			}
		}
	}
}

FInteractableData ANPCCharacter::GetInteractableData_Implementation()
{
	FInteractableData Data;

	Data.InteractableType = EInteractableType::NonPlayerCharacter;
	Data.Name = FText::FromString(NPCName); 
	Data.Action = FText::FromString(TEXT("대화하기")); 
	Data.InteractionDuration = 0.0f; 

	return Data;
}

float ANPCCharacter::GetInteractionDistance_Implementation()
{
	return 200.0f;
}

bool ANPCCharacter::SellItemToPlayer(FName ItemID, int32 Quantity)
{
	// 1. 플레이어 가져오기
	APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (!Player) return false;

	UInventoryComponent* PlayerInv = Player->GetInventory();
	if (!PlayerInv) return false;

	// 2. 아이템 데이터 로드

	FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemID, TEXT("Shop"));
	if (!ItemData)
	{
		UE_LOG(LogTemp, Error, TEXT("Item %s not found in DataTable"), *ItemID.ToString());
		return false;
	}


	// 3. 아이템 생성
	UItemBase* NewItem = NewObject<UItemBase>();
	NewItem->ID = ItemData->ID;
	NewItem->ItemType = ItemData->ItemType;
	NewItem->ItemQuality = ItemData->ItemQuality;
	NewItem->NumericData = ItemData->NumericData;
	NewItem->TextData = ItemData->TextData;
	NewItem->AssetData = ItemData->AssetData;
	NewItem->ItemStatistics = ItemData->ItemStatistics;
	NewItem->Quantity = Quantity;

	// 4. InventoryComponent의 HandleAddItem 사용
	FItemAddResult Result = PlayerInv->HandleAddItem(NewItem);

	// 5. 결과 처리
	if (Result.OperationResult == EItemAddResult::IAR_AllItemAdded)
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully sold %d x %s to player"),
			Quantity, *ItemData->TextData.Name.ToString());
		return true;
	}
	else if (Result.OperationResult == EItemAddResult::IAR_PartialAmountItemAdded)
	{
		UE_LOG(LogTemp, Warning, TEXT("Partial sale: %d / %d added"), 
			Result.ActualAmountAdded, Quantity);
		return true; // 부분 성공도 성공으로 처리
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to add item to inventory: %s"),
			*Result.ResultMessage.ToString());
		return false;
	}

}

bool ANPCCharacter::BuyItemFromPlayer(UItemBase* Item, int32 Quantity, int32& OutGoldReceived)
{
	if (!Item || Quantity <= 0) return false;

	// 판매 불가 아이템 체크 (SellValue == 0)
	if (Item->ItemStatistics.SellValue <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item cannot be sold: %s"),
			*Item->TextData.Name.ToString());
		return false;
	}

	APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (!Player) return false;

	UInventoryComponent* PlayerInv = Player->GetInventory();
	if (!PlayerInv) return false;

	// 플레이어 인벤토리에서 아이템 찾기
	UItemBase* PlayerItem = PlayerInv->FindMatchingItem(Item);
	if (!PlayerItem || PlayerItem->Quantity < Quantity)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player doesn't have enough items"));
		return false;
	}

	// 구매 가격 계산 (판매 가격의 80%)
	int32 BuyBackPrice = FMath::FloorToInt(Item->ItemStatistics.SellValue * 0.8f);
	int32 TotalPrice = BuyBackPrice * Quantity;

	// 플레이어 인벤토리에서 제거
	int32 RemovedAmount = PlayerInv->RemoveAmountOfItem(PlayerItem, Quantity);

	if (RemovedAmount > 0)
	{
		// TODO: 골드 지급
		OutGoldReceived = BuyBackPrice * RemovedAmount;

		UE_LOG(LogTemp, Log, TEXT("NPC bought %d x %s from player for %d gold"),
			RemovedAmount, *Item->TextData.Name.ToString(), TotalPrice);

		return true;
	}

	return false;

}

int32 ANPCCharacter::GetItemPrice(FName ItemID) const
{

	FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemID, TEXT("Shop"));
	if (!ItemData) return 0;

	return FMath::CeilToInt(ItemData->ItemStatistics.SellValue * PriceMultiplier);
}

FItemDataRow* ANPCCharacter::GetItemData(FName ItemID) const
{
	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("ItemDataTable is not set!"));
		return nullptr;
	}

	FItemDataRow* Row = ItemDataTable->FindRow<FItemDataRow>(ItemID, TEXT("NPCShop"));

	if (!Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item '%s' not found in DataTable"), *ItemID.ToString());
	}

	return Row;
}

ANPCAIController* ANPCCharacter::GetNPCAIController() const
{
	return Cast<ANPCAIController>(GetController());
}

void ANPCCharacter::OnDialogueStart(const FDialogueRow& DialogueData)
{
	if (!DialogueComponent) return;

	if (!DialogueWidget)
	{
		if (!DialogueWidgetClass) return;

		UE_LOG(LogTemp, Log, TEXT("[%s] Creating new DialogueWidget"), *NPCName);
		DialogueWidget = CreateWidget<UDialogueWidget>(GetWorld(), DialogueWidgetClass);

		if (!DialogueWidget) return;

		// 옵션 클릭 이벤트 바인딩
		DialogueWidget->OnOptionClicked.AddDynamic(this, &ANPCCharacter::OnOptionSelected);
	}
	DialogueWidget->SetDialogueComponent(DialogueComponent);

	// Widget 표시 및 업데이트
	if (!DialogueWidget->IsInViewport())
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Adding DialogueWidget to viewport"), *NPCName);
		DialogueWidget->AddToViewport(100);
	}
	DialogueWidget->ShowDialogue(DialogueData);

}
void ANPCCharacter::OpenShop()
{
	if (!ShopWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] ShopWidgetClass not set"), *NPCName);
		return;
	}

	if (!ShopWidget)
	{
		ShopWidget = CreateWidget<UShopWidget>(GetWorld(), ShopWidgetClass);
		if (!ShopWidget)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Failed to create shop widget"), *NPCName);
			return;
		}
	}
	ShopWidget->AddToViewport(100);

	ShopWidget->InitializeShop(this);
	UE_LOG(LogTemp, Log, TEXT("[%s] Shop opened"), *NPCName);
}


void ANPCCharacter::OnDialogueUpdate(const FDialogueRow& DialogueData)
{
	if (DialogueWidget && DialogueWidget->IsInViewport())
	{
		DialogueWidget->ShowDialogue(DialogueData);
	}
}

void ANPCCharacter::OnDialogueEnd()
{
	bool bShopIsOpen = (ShopWidget && ShopWidget->IsInViewport());
	// Widget 정리
	if (DialogueWidget && DialogueWidget->IsInViewport())
	{
		DialogueWidget->RemoveFromParent();
		// Widget은 재사용을 위해 유지 (nullptr 안 함)
	}

	// 상태 정리
	bIsInteracting = false;
	AActor* PreviousInteractor = CurrentInteractor;
	CurrentInteractor = nullptr;

	if (!bShopIsOpen)
	{
		if (PreviousInteractor)
		{
			if (APlayerController* PC = Cast<APlayerController>(PreviousInteractor->GetInstigatorController()))
			{
				PC->SetShowMouseCursor(false);
				FInputModeGameOnly InputMode;
				PC->SetInputMode(InputMode);
				UE_LOG(LogTemp, Log, TEXT("[%s] Player input mode restored"), *NPCName);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Shop is open, skipping input mode restore"), *NPCName);
	}

	// AI 상태 복구 (상점 열릴 때는 스킵)
	if (!bShopIsOpen)
	{
		if (ANPCAIController* AIController = GetNPCAIController())
		{
			AIController->SetInteractionTarget(nullptr);
			// 플레이어가 여전히 근처에 있으면 Alert
			if (AIController->GetTargetActor())
			{
				AIController->SetNPCState(ENPCMode::Alert);
			}
			else
			{
				AIController->SetNPCState(ENPCMode::Idle);
			}
		}

		// 블루프린트 이벤트
		OnInteractionEnded(PreviousInteractor);
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] Dialogue ended"), *NPCName);
}

void ANPCCharacter::OnOptionSelected(int32 OptionIndex)
{
	if (DialogueComponent)
	{
		DialogueComponent->SelectOption(OptionIndex);
	}
}
