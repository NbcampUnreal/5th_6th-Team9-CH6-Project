// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NPCCharacter.h"
#include "NPC/NPCCharacter_Quest.h"
#include "NPC/NPCAIController.h"
#include "NPC/DialogueComponent.h"
#include "NPC/DialogueWidget.h"
#include "Items/ItemBase.h"
#include "Inventory/InventoryComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ShopWidget.h"
#include "Blueprint/UserWidget.h"
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
	NPCName = TEXT("NPC");
	bIsInteracting = false;
	CurrentInteractor = nullptr;
	LastNPCState = 0;
	DialogueWidget = nullptr;
	ShopWidget = nullptr;
	float RestockTime = 600.f;
}

// Called when the game starts or when spawned
void ANPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 거래 NPC일 때만 상점 초기화
	if (!Cast<ANPCCharacter_Quest>(this))
	{
		InitializeShopItems();
	}

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
		DialogueComponent->OnDialogueStarted.AddUniqueDynamic(this, &ANPCCharacter::OnDialogueStart);
		DialogueComponent->OnDialogueUpdated.AddUniqueDynamic(this, &ANPCCharacter::OnDialogueUpdate);
		DialogueComponent->OnDialogueEnded.AddUniqueDynamic(this, &ANPCCharacter::OnDialogueEnd);
		// OnOptionSelected는 DialogueWidget에서 처리
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

	ShopItemList.Empty();

	TArray<FName> ShopItems = {
		FName(TEXT("600001")),  // 최하급 체력회복물약
		FName(TEXT("600002")),   // 최하급 마나회복물약
		FName(TEXT("700003"))
	};

	UE_LOG(LogTemp, Warning, TEXT("[%s] Initializing shop (%d items)"),
		*NPCName, ShopItems.Num());

	for (const FName& ItemRowName : ShopItems)
	{
		FString ContextString;
		const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemRowName, ContextString);

		if (ItemData)
		{
			FShopItemData NewShopItem;
			NewShopItem.ItemRowName = ItemRowName;
			NewShopItem.CurrentStock = 99;
			NewShopItem.MaxStock = 99;

			ShopItemList.Add(NewShopItem);

			UE_LOG(LogTemp, Log, TEXT("  [%s] Price: %dG, Sell: %dG"),
				*ItemData->TextData.Name.ToString(),
				FMath::FloorToInt(ItemData->ItemStatistics.SellValue),
				FMath::FloorToInt(ItemData->ItemStatistics.SellValue * 0.8f));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("  Item not found in DataTable: %s"),
				*ItemRowName.ToString());
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[%s] Shop initialized: %d items"), *NPCName, ShopItemList.Num());
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
		if (CurrentState == static_cast<uint8>(ENPCMode::Alert) &&
			LastNPCState == static_cast<uint8>(ENPCMode::Idle))
		{
			AActor* Target = AIController->GetTargetActor();
			if (Target)
			{
				OnPlayerDetected(Target);
			}
		}
		else if (CurrentState == static_cast<uint8>(ENPCMode::Idle) &&
			LastNPCState == static_cast<uint8>(ENPCMode::Alert))
		{
			OnPlayerLost();
		}
		LastNPCState = CurrentState;
	}
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

bool ANPCCharacter::SellItemToPlayer(APlayerCharacter_SB* Player, FName ItemRowName, int32 Quantity)
{
	if (!Player || !ItemDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] SellItemToPlayer: Invalid params"), *NPCName);
		return false;
	}

	// 아이템 데이터 조회
	FString ContextString;
	const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemRowName, ContextString);

	if (!ItemData)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Item not found: %s"),
			*NPCName, *ItemRowName.ToString());
		return false;
	}

	UInventoryComponent* PlayerInventory = Player->GetInventory();
	if (!PlayerInventory)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Player has no inventory!"), *NPCName);
		return false;
	}

	// 새 아이템 생성
	UItemBase* NewItem = NewObject<UItemBase>(PlayerInventory, UItemBase::StaticClass());
	if (!NewItem)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to create item!"), *NPCName);
		return false;
	}

	// 3. 아이템 생성
	NewItem->ID = ItemData->ID;
	NewItem->ItemType = ItemData->ItemType;
	NewItem->ItemQuality = ItemData->ItemQuality;
	NewItem->NumericData = ItemData->NumericData;
	NewItem->TextData = ItemData->TextData;
	NewItem->AssetData = ItemData->AssetData;
	NewItem->ItemStatistics = ItemData->ItemStatistics;
	NewItem->EquipWeaponClass = ItemData->EquipWeaponClass;
	NewItem->Quantity = Quantity;

	// 4. InventoryComponent의 HandleAddItem 사용
	const FItemAddResult AddResult = PlayerInventory->HandleAddItem_AutoHotbarFirst(NewItem);

	// 5. 결과 처리
	if (AddResult.OperationResult == EItemAddResult::IAR_AllItemAdded)
	{
		// ============================================
		// ItemStatistics.SellValue 사용
		// ============================================
		UE_LOG(LogTemp, Log, TEXT("[%s] Sold %dx %s to player for %fG"),
			*NPCName,
			Quantity,
			*ItemData->TextData.Name.ToString(),
			ItemData->ItemStatistics.SellValue * Quantity);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to add item: %s"),
			*NPCName, *AddResult.ResultMessage.ToString());
		return false;
	}
}

bool ANPCCharacter::BuyItemFromPlayer(APlayerCharacter_SB* Player, UItemBase* Item, int32 Quantity)
{
	if (!Player || !Item) return false;

	// 판매 불가 아이템 체크 (SellValue == 0)
	if (Item->ItemStatistics.SellValue <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item cannot be sold: %s"),
			*Item->TextData.Name.ToString());
		return false;
	}

	// 석판 체크
	FString ItemName = Item->TextData.Name.ToString();
	if (ItemName.Contains(TEXT("석판")) || ItemName.Contains(TEXT("Stone")))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Cannot buy stone tablet: %s"),
			*NPCName, *ItemName);
		return false;
	}

	UInventoryComponent* PlayerInventory = Player->GetInventory();
	if (!PlayerInventory)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Player has no inventory!"), *NPCName);
		return false;
	}

	// 인벤토리에서 제거
	const int32 ActualRemoved = PlayerInventory->RemoveAmountOfItem(Item, Quantity);

	if (ActualRemoved <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Nothing was removed from inventory!"), *NPCName);
		return false;
	}
	// ============================================
	// ItemStatistics.SellValue 사용 
	// ============================================
	const int32 SellPrice = FMath::FloorToInt(Item->ItemStatistics.SellValue * 0.8f);
	const int32 TotalGold = SellPrice * ActualRemoved;

	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] ItemDataTable is NULL!"), *NPCName);
		return false;
	}

	// 기존 ModifyGold 대신 골드 아이템으로 지급
	FName GoldItemRowName = FName(TEXT("700001"));
	FString ContextString;
	const FItemDataRow* GoldItemData = ItemDataTable->FindRow<FItemDataRow>(GoldItemRowName, ContextString);

	if (GoldItemData)
	{
		UItemBase* GoldItem = NewObject<UItemBase>(PlayerInventory, UItemBase::StaticClass());
		GoldItem->ID = GoldItemData->ID;
		GoldItem->ItemType = GoldItemData->ItemType;
		GoldItem->ItemQuality = GoldItemData->ItemQuality;
		GoldItem->NumericData = GoldItemData->NumericData;
		GoldItem->TextData = GoldItemData->TextData;
		GoldItem->AssetData = GoldItemData->AssetData;
		GoldItem->ItemStatistics = GoldItemData->ItemStatistics;
		GoldItem->Quantity = TotalGold;

		PlayerInventory->HandleAddItem_AutoHotbarFirst(GoldItem);

		UE_LOG(LogTemp, Log, TEXT("[%s] Gave %dx Gold item to player"), *NPCName, TotalGold);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Gold item 700001 not found in DataTable!"), *NPCName);
		return false;
	}
	return true;
}

int32 ANPCCharacter::GetItemPrice(FName ItemRowName) const
{
	if (!ItemDataTable)
	{
		return 0;
	}
	FString ContextString;
	const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemRowName, ContextString);

	if (ItemData)
	{
		return ItemData->ItemStatistics.SellValue;
	}

	return 0;
}

FItemDataRow* ANPCCharacter::GetItemData(FName ItemRowName) const
{
	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("ItemDataTable is not set!"));
		return nullptr;
	}

	FString ContextString;
	FItemDataRow* Row = ItemDataTable->FindRow<FItemDataRow>(ItemRowName, ContextString);

	if (!Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item '%s' not found in DataTable"), *ItemRowName.ToString());
	}

	return Row;
}

void ANPCCharacter::OpenShop()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!ShopWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] ShopWidgetClass not set"), *NPCName);
		return;
	}

	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] No PlayerController found"), *NPCName);
		return;
	}

	if (!ShopWidget)
	{
		ShopWidget = CreateWidget<UShopWidget>(PC, ShopWidgetClass);
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
		DialogueWidget->OnOptionClicked.AddUniqueDynamic(this, &ANPCCharacter::OnOptionSelected);
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
	// 퀘스트 완료 팝업 떠있는지 체크
	bool bQuestPopupOpen = false;
	ANPCCharacter_Quest* QuestNPC = Cast<ANPCCharacter_Quest>(this);
	if (QuestNPC && QuestNPC->ActiveQuestCompleteWidget
		&& QuestNPC->ActiveQuestCompleteWidget->IsInViewport())
	{
		bQuestPopupOpen = true;
	}

	if (DialogueWidget && DialogueWidget->IsInViewport())
	{
		DialogueWidget->RemoveFromParent();
	}

	// 상태 정리
	bIsInteracting = false;
	AActor* PreviousInteractor = CurrentInteractor;
	CurrentInteractor = nullptr;

	if (!bShopIsOpen && !bQuestPopupOpen)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			PC->SetShowMouseCursor(false);
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
		}
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
