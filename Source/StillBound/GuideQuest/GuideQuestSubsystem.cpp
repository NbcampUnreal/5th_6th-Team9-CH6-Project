#include "GuideQuestSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Character/PlayerCharacter_SB.h"
#include "GameFramework/PlayerController.h"
#include "GameInstance/SBGameInstance.h"
#include "Inventory/InventoryComponent.h"
#include "Data/SBWorldSaveGame.h"
#include "Items/ItemBase.h"
#include "Data/ItemData.h"

DEFINE_LOG_CATEGORY(LogGuideQuest);

void UGuideQuestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const USBGameInstance* SBGI = Cast<USBGameInstance>(GI))
		{
			InitializeQuestTables(SBGI->GuideQuestMasterTable, SBGI->GuideQuestObjectiveTable);
		}
	}
}

void UGuideQuestSubsystem::Deinitialize()
{
	if (ActiveWidget.IsValid())
	{
		ActiveWidget->RemoveFromParent();
		ActiveWidget.Reset();
	}

	ActiveQuestId = NAME_None;
	ActiveQuestRow = nullptr;
	ActiveObjectives.Reset();
	CompletedQuestIds.Reset();

	Super::Deinitialize();
}

void UGuideQuestSubsystem::InitializeQuestTables(UDataTable* InMasterTable, UDataTable* InObjectiveTable)
{
	GuideQuestMasterTable = InMasterTable;
	GuideQuestObjectiveTable = InObjectiveTable;

	//디버깅용 코드
	UE_LOG(LogTemp, Warning, TEXT("[GuideQuestSubsystem] InitializeQuestTables called"));
	UE_LOG(LogTemp, Warning, TEXT("[GuideQuestSubsystem] MasterTable = %s"),
		GuideQuestMasterTable ? TEXT("Valid") : TEXT("Null"));
	UE_LOG(LogTemp, Warning, TEXT("[GuideQuestSubsystem] ObjectiveTable = %s"),
		GuideQuestObjectiveTable ? TEXT("Valid") : TEXT("Null"));
}

bool UGuideQuestSubsystem::StartQuest(FName QuestId, bool bResetProgress)
{
	//디버깅용 코드
	UE_LOG(LogTemp, Warning, TEXT("[GuideQuestSubsystem] StartQuest called: %s"), *QuestId.ToString());

	if (!IsValid(GuideQuestMasterTable) || !IsValid(GuideQuestObjectiveTable))
	{
		UE_LOG(LogGuideQuest, Error, TEXT("[GuideQuest] StartQuest failed: DataTables are not initialized."));
		return false;
	}

	if (QuestId.IsNone())
	{
		UE_LOG(LogGuideQuest, Warning, TEXT("[GuideQuest] StartQuest skipped: QuestId is None."));
		return false;
	}

	const FGuideQuestMasterRow* FoundMasterRow =
		GuideQuestMasterTable->FindRow<FGuideQuestMasterRow>(QuestId, TEXT("GuideQuestMasterLookup"), true);

	if (!FoundMasterRow)
	{
		UE_LOG(LogGuideQuest, Error, TEXT("[GuideQuest] StartQuest failed: Master row not found for %s"), *QuestId.ToString());
		return false;
	}

	TArray<FGuideQuestRuntimeObjective> LoadedObjectives;
	if (!BuildObjectivesForQuest(QuestId, LoadedObjectives))
	{
		UE_LOG(LogGuideQuest, Error, TEXT("[GuideQuest] StartQuest failed: Objective rows not found for %s"), *QuestId.ToString());
		return false;
	}

	const FName PreviousQuestId = ActiveQuestId;

	ActiveQuestRow = FoundMasterRow;

	if (bResetProgress || PreviousQuestId != QuestId || ActiveObjectives.Num() != LoadedObjectives.Num())
	{
		ActiveObjectives = MoveTemp(LoadedObjectives);
	}

	ActiveQuestId = QuestId;

	//디버깅용 코드
	UE_LOG(LogTemp, Warning, TEXT("[GuideQuestSubsystem] StartQuest success: %s"), *ActiveQuestId.ToString());

	OnGuideQuestUpdated.Broadcast();
	return true;
}

bool UGuideQuestSubsystem::BuildObjectivesForQuest(FName QuestId, TArray<FGuideQuestRuntimeObjective>& OutObjectives) const
{
	OutObjectives.Reset();

	if (!IsValid(GuideQuestObjectiveTable) || QuestId.IsNone())
	{
		return false;
	}

	TArray<FGuideQuestObjectiveRow*> ObjectiveRows;
	GuideQuestObjectiveTable->GetAllRows(TEXT("GuideQuestObjectiveLoad"), ObjectiveRows);

	for (const FGuideQuestObjectiveRow* Row : ObjectiveRows)
	{
		if (!Row || Row->QuestId != QuestId)
		{
			continue;
		}

		FGuideQuestRuntimeObjective RuntimeObjective;
		RuntimeObjective.EventType = Row->EventType;
		RuntimeObjective.TargetId = Row->TargetId;
		RuntimeObjective.RequiredCount = Row->RequiredCount;
		RuntimeObjective.CurrentCount = 0;
		RuntimeObjective.Description = Row->Description;
		OutObjectives.Add(RuntimeObjective);
	}

	return OutObjectives.Num() > 0;
}

bool UGuideQuestSubsystem::GiveReward(APlayerCharacter_SB* PlayerCharacter)
{
	if (!PlayerCharacter || !ActiveQuestRow)
	{
		return false;
	}

	// 골드 보상
	if (ActiveQuestRow->RewardType == EGuideQuestRewardType::Gold)
	{
		if (ActiveQuestRow->RewardGold <= 0)
		{
			return false;
		}

		PlayerCharacter->ModifyGold(ActiveQuestRow->RewardGold);
		return true;
	}

	// 아이템 보상
	if (ActiveQuestRow->RewardType == EGuideQuestRewardType::Item)
	{
		if (ActiveQuestRow->RewardItemId.IsNone() || ActiveQuestRow->RewardItemCount <= 0)
		{
			return false;
		}

		UInventoryComponent* Inventory = PlayerCharacter->GetInventory();
		if (!Inventory)
		{
			return false;
		}

		UItemBase* RewardItem = Inventory->CreateItemInstanceByID(
			ActiveQuestRow->RewardItemId,
			ActiveQuestRow->RewardItemCount
		);

		if (!RewardItem)
		{
			return false;
		}

		const FItemAddResult AddResult = Inventory->HandleAddItem_AutoHotbarFirst(RewardItem);
		return AddResult.ActualAmountAdded > 0;
	}

	// None 보상
	return true;
}
void UGuideQuestSubsystem::EnsureStarted(FName FirstQuestId)
{
	//디버깅용 코드
	UE_LOG(LogTemp, Warning, TEXT("[GuideQuestSubsystem] EnsureStarted called: %s"), *FirstQuestId.ToString());

	if (!ActiveQuestId.IsNone())
	{
		//디버깅용 코드
		UE_LOG(LogTemp, Warning, TEXT("[GuideQuestSubsystem] ActiveQuest already exists: %s"), *ActiveQuestId.ToString());
		return;
	}

	StartQuest(FirstQuestId, true);
}

void UGuideQuestSubsystem::ReportProgress(EGuideQuestEventType EventType, FName TargetId, int32 DeltaCount)
{
	//디버깅용 코드
	UE_LOG(LogTemp, Warning, TEXT("[GuideQuest] ReportProgress called: EventType=%d TargetId=%s Delta=%d"),
		(int32)EventType, *TargetId.ToString(), DeltaCount);

	if (ActiveQuestId.IsNone() || !ActiveQuestRow || DeltaCount <= 0)
	{
		return;
	}

	bool bAnyChanged = false;

	for (FGuideQuestRuntimeObjective& Objective : ActiveObjectives)
	{
		//디버깅용 코드
		UE_LOG(LogTemp, Warning, TEXT("[GuideQuest] Checking Objective: EventType=%d TargetId=%s Current=%d Required=%d"),
			(int32)Objective.EventType,
			*Objective.TargetId.ToString(),
			Objective.CurrentCount,
			Objective.RequiredCount);

		if (!Objective.Matches(EventType, TargetId))
		{
			continue;
		}

		if (Objective.CurrentCount >= Objective.RequiredCount)
		{
			continue;
		}

		const int32 PrevCount = Objective.CurrentCount;
		Objective.CurrentCount = FMath::Clamp(Objective.CurrentCount + DeltaCount, 0, Objective.RequiredCount);
		bAnyChanged |= (PrevCount != Objective.CurrentCount);

		//디버깅용 코드
		UE_LOG(LogTemp, Warning, TEXT("[GuideQuest] Objective matched: %d -> %d / %d"),
			PrevCount,
			Objective.CurrentCount,
			Objective.RequiredCount);
	}

	if (!bAnyChanged)
	{
		//디버깅용 코드
		UE_LOG(LogTemp, Warning, TEXT("[GuideQuest] No objective changed"));
		return;
	}

	OnGuideQuestUpdated.Broadcast();

	//디버깅용 코드
	UE_LOG(LogTemp, Warning, TEXT("[GuideQuest] IsQuestComplete = %s"),
		IsQuestComplete() ? TEXT("true") : TEXT("false"));

	if (IsQuestComplete())
	{
		CompleteActiveQuest();
	}
}

void UGuideQuestSubsystem::ReportCollectItem(FName ItemId, int32 DeltaCount)
{
	//디버깅용 코드
	UE_LOG(LogTemp, Warning, TEXT("[GuideQuest] ReportCollectItem received: %s x%d"),
		*ItemId.ToString(), DeltaCount);

	ReportProgress(EGuideQuestEventType::CollectItem, ItemId, DeltaCount); 
}

void UGuideQuestSubsystem::ReportBuildPlaced(FName BuildingId, int32 DeltaCount) 
{
	ReportProgress(EGuideQuestEventType::BuildPlaced, BuildingId, DeltaCount); 
}

void UGuideQuestSubsystem::ReportCraftItem(FName ItemId, int32 DeltaCount) 
{
	ReportProgress(EGuideQuestEventType::CraftItem, ItemId, DeltaCount); 
}

void UGuideQuestSubsystem::ReportSellItem(FName ItemId, int32 DeltaCount) 
{
	ReportProgress(EGuideQuestEventType::SellItem, ItemId, DeltaCount); 
}

void UGuideQuestSubsystem::ReportBuyItem(FName ItemId, int32 DeltaCount)
{ 
	ReportProgress(EGuideQuestEventType::BuyItem, ItemId, DeltaCount);
}

void UGuideQuestSubsystem::ReportKillEnemy(FName EnemyId, int32 DeltaCount)
{ 
	ReportProgress(EGuideQuestEventType::KillEnemy, EnemyId, DeltaCount); 
}

void UGuideQuestSubsystem::ReportKillBoss(FName BossId)
{ 
	ReportProgress(EGuideQuestEventType::KillBoss, BossId, 1);
}

bool UGuideQuestSubsystem::IsQuestComplete() const
{
	if (!ActiveQuestRow || ActiveObjectives.Num() <= 0)
	{
		return false;
	}

	for (const FGuideQuestRuntimeObjective& Objective : ActiveObjectives)
	{
		if (Objective.CurrentCount < Objective.RequiredCount)
		{
			return false;
		}
	}

	return true;
}

void UGuideQuestSubsystem::CompleteActiveQuest()
{
	//디버깅용 코드
	UE_LOG(LogTemp, Warning, TEXT("[GuideQuest] CompleteActiveQuest called"));

	if (!ActiveQuestRow)
	{
		UE_LOG(LogTemp, Error, TEXT("[GuideQuest] ActiveQuestRow is null"));
		return;
	}

	const FName CompletedQuestId = ActiveQuestId;

	if (!CompletedQuestIds.Contains(CompletedQuestId))
	{
		CompletedQuestIds.Add(CompletedQuestId);
	}

	//보상 지급
	const bool bRewardSuccess = GiveReward(GetPlayerCharacter());
	if (!bRewardSuccess)
	{
		UE_LOG(LogGuideQuest, Warning, TEXT("[GuideQuest] Reward failed for quest %s"), *CompletedQuestId.ToString());
	}

	//다음 퀘스트 ID는 초기화 전에 미리 백업
	const FName NextQuestId = ActiveQuestRow->NextQuestId;

	//디버깅용 코드
	UE_LOG(LogTemp, Warning, TEXT("[GuideQuest] CompletedQuestId = %s"), *CompletedQuestId.ToString());
	UE_LOG(LogTemp, Warning, TEXT("[GuideQuest] NextQuestId = %s"), *NextQuestId.ToString());

	//현재 퀘스트 상태 정리
	ActiveQuestId = NAME_None;
	ActiveQuestRow = nullptr;
	ActiveObjectives.Reset();

	//완료 이벤트 먼저 브로드캐스트
	OnGuideQuestCompleted.Broadcast(CompletedQuestId);

	//다음 퀘스트가 있으면 바로 시작
	// StartQuest 내부에서 OnGuideQuestUpdated.Broadcast()가 호출되므로
	// 여기서 중복으로 Updated를 호출하지 않음
	if (!NextQuestId.IsNone())
	{
		//디버깅용 코드
		UE_LOG(LogTemp, Warning, TEXT("[GuideQuest] Starting next quest: %s"), *NextQuestId.ToString());

		StartQuest(NextQuestId, true);
		return;
	}

	//다음 퀘스트가 없을 때만 UI 갱신
	OnGuideQuestUpdated.Broadcast();
}

void UGuideQuestSubsystem::ExportToSaveGame(USBWorldSaveGame* SaveGameObject) const
{
	if (!IsValid(SaveGameObject))
	{
		return;
	}

	SaveGameObject->bHasGuideQuest = !ActiveQuestId.IsNone() || CompletedQuestIds.Num() > 0;
	SaveGameObject->SavedGuideQuest.bHasActiveQuest = !ActiveQuestId.IsNone();
	SaveGameObject->SavedGuideQuest.ActiveQuestId = ActiveQuestId;
	SaveGameObject->SavedGuideQuest.CompletedQuestIds = CompletedQuestIds;

	SaveGameObject->SavedGuideQuest.ActiveObjectiveCounts.Reset();
	for (const FGuideQuestRuntimeObjective& Objective : ActiveObjectives)
	{
		SaveGameObject->SavedGuideQuest.ActiveObjectiveCounts.Add(Objective.CurrentCount);
	}
}

void UGuideQuestSubsystem::ImportFromSaveGame(const USBWorldSaveGame* SaveGameObject)
{
	if (!IsValid(SaveGameObject) || !SaveGameObject->bHasGuideQuest)
	{
		return;
	}

	CompletedQuestIds = SaveGameObject->SavedGuideQuest.CompletedQuestIds;

	if (!SaveGameObject->SavedGuideQuest.bHasActiveQuest || SaveGameObject->SavedGuideQuest.ActiveQuestId.IsNone())
	{
		ActiveQuestId = NAME_None;
		ActiveQuestRow = nullptr;
		ActiveObjectives.Reset();
		OnGuideQuestUpdated.Broadcast();
		return;
	}

	if (!StartQuest(SaveGameObject->SavedGuideQuest.ActiveQuestId, true))
	{
		UE_LOG(LogGuideQuest, Error, TEXT("[GuideQuest] Import failed: StartQuest(%s) failed."),
			*SaveGameObject->SavedGuideQuest.ActiveQuestId.ToString());
		return;
	}

	if (ActiveObjectives.Num() == SaveGameObject->SavedGuideQuest.ActiveObjectiveCounts.Num())
	{
		for (int32 i = 0; i < ActiveObjectives.Num(); ++i)
		{
			ActiveObjectives[i].CurrentCount = FMath::Clamp(
				SaveGameObject->SavedGuideQuest.ActiveObjectiveCounts[i],
				0,
				ActiveObjectives[i].RequiredCount);
		}
	}

	OnGuideQuestUpdated.Broadcast();
}

void UGuideQuestSubsystem::AttachWidget(APlayerController* PlayerController, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!IsValid(PlayerController) || !WidgetClass)
	{
		UE_LOG(LogGuideQuest, Warning, TEXT("[GuideQuest] AttachWidget failed: invalid arguments"));
		return;
	}

	if (ActiveWidget.IsValid())
	{
		return;
	}

	UUserWidget* NewWidget = CreateWidget<UUserWidget>(PlayerController, WidgetClass);
	if (!IsValid(NewWidget))
	{
		UE_LOG(LogGuideQuest, Error, TEXT("[GuideQuest] AttachWidget failed: CreateWidget returned null"));
		return;
	}

	NewWidget->AddToViewport(30);
	ActiveWidget = NewWidget;
	OnGuideQuestUpdated.Broadcast();
}

APlayerCharacter_SB* UGuideQuestSubsystem::GetPlayerCharacter() const
{
	const UGameInstance* GI = GetGameInstance();
	if (!IsValid(GI))
	{
		return nullptr;
	}

	UWorld* World = GI->GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!IsValid(PC))
	{
		return nullptr;
	}

	return Cast<APlayerCharacter_SB>(PC->GetPawn());
}

bool UGuideQuestSubsystem::HasActiveQuest() const
{
	return ActiveQuestRow != nullptr && !ActiveQuestId.IsNone();
}

FText UGuideQuestSubsystem::GetActiveQuestTitleText() const
{
	return ActiveQuestRow ? ActiveQuestRow->Title : FText::GetEmpty();
}

FText UGuideQuestSubsystem::GetActiveQuestSummaryText() const
{
	return ActiveQuestRow ? ActiveQuestRow->Summary : FText::GetEmpty();
}

FText UGuideQuestSubsystem::BuildObjectiveProgressText() const
{
	FString Combined;
	for (const FGuideQuestRuntimeObjective& Objective : ActiveObjectives)
	{
		const FString Prefix = (Objective.CurrentCount >= Objective.RequiredCount) ? TEXT("[완료] ") : TEXT("[진행] ");
		const FString Line = FString::Printf(TEXT("%s%s (%d/%d)"),
			*Prefix, *Objective.Description.ToString(), Objective.CurrentCount, Objective.RequiredCount);

		if (!Combined.IsEmpty()) Combined += TEXT("\n");
		Combined += Line;
	}
	return FText::FromString(Combined);
}

FText UGuideQuestSubsystem::BuildRewardText() const
{
	if (!ActiveQuestRow)
	{
		return FText::GetEmpty();
	}

	switch (ActiveQuestRow->RewardType)
	{
	case EGuideQuestRewardType::Gold:
		return FText::FromString(FString::Printf(TEXT("보상: 골드 %d"), ActiveQuestRow->RewardGold));

	case EGuideQuestRewardType::Item:
	{
		FText RewardItemName = FText::FromName(ActiveQuestRow->RewardItemId);

		// 플레이어 인벤토리의 ItemDataTable을 이용해 표시 이름 찾기
		if (APlayerCharacter_SB* PlayerCharacter = GetPlayerCharacter())
		{
			if (UInventoryComponent* Inventory = PlayerCharacter->GetInventory())
			{
				if (IsValid(Inventory->ItemDataTable))
				{
					if (const FItemDataRow* ItemRow =
						Inventory->ItemDataTable->FindRow<FItemDataRow>(ActiveQuestRow->RewardItemId, TEXT("GuideQuestRewardLookup")))
					{
						RewardItemName = ItemRow->TextData.Name;
					}
				}
			}
		}

		return FText::Format(
			FText::FromString(TEXT("보상: 아이템 {0} x{1}")),
			RewardItemName,
			FText::AsNumber(ActiveQuestRow->RewardItemCount)
		);
	}

	default:
		return FText::FromString(TEXT("보상: 없음"));
	}
}