#include "GuideQuestSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Character/PlayerCharacter_SB.h"
#include "GameFramework/PlayerController.h"
#include "GameInstance/SBGameInstance.h"
#include "Inventory/InventoryComponent.h"
#include "Data/SBWorldSaveGame.h"

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

bool UGuideQuestSubsystem::StartQuest(FName QuestId, bool bResetProgress)
{
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

	ActiveQuestId = QuestId;
	ActiveQuestRow = FoundMasterRow;

	if (bResetProgress || ActiveQuestId != QuestId || ActiveObjectives.Num() != LoadedObjectives.Num())
	{
		ActiveObjectives = MoveTemp(LoadedObjectives);
	}

	OnGuideQuestUpdated.Broadcast();
	return true;
}

void UGuideQuestSubsystem::EnsureStarted(FName FirstQuestId)
{
	if (!ActiveQuestId.IsNone())
	{
		return;
	}

	StartQuest(FirstQuestId, true);
}

void UGuideQuestSubsystem::ReportProgress(EGuideQuestEventType EventType, FName TargetId, int32 DeltaCount)
{
	if (ActiveQuestId.IsNone() || !ActiveQuestRow || DeltaCount <= 0)
	{
		return;
	}

	bool bAnyChanged = false;

	for (FGuideQuestRuntimeObjective& Objective : ActiveObjectives)
	{
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
	}

	if (!bAnyChanged)
	{
		return;
	}

	OnGuideQuestUpdated.Broadcast();

	if (IsQuestComplete())
	{
		CompleteActiveQuest();
	}
}

void UGuideQuestSubsystem::ReportCollectItem(FName ItemId, int32 DeltaCount) { ReportProgress(EGuideQuestEventType::CollectItem, ItemId, DeltaCount); }
void UGuideQuestSubsystem::ReportBuildPlaced(FName BuildingId, int32 DeltaCount) { ReportProgress(EGuideQuestEventType::BuildPlaced, BuildingId, DeltaCount); }
void UGuideQuestSubsystem::ReportCraftItem(FName ItemId, int32 DeltaCount) { ReportProgress(EGuideQuestEventType::CraftItem, ItemId, DeltaCount); }
void UGuideQuestSubsystem::ReportSellItem(FName ItemId, int32 DeltaCount) { ReportProgress(EGuideQuestEventType::SellItem, ItemId, DeltaCount); }
void UGuideQuestSubsystem::ReportBuyItem(FName ItemId, int32 DeltaCount) { ReportProgress(EGuideQuestEventType::BuyItem, ItemId, DeltaCount); }
void UGuideQuestSubsystem::ReportKillEnemy(FName EnemyId, int32 DeltaCount) { ReportProgress(EGuideQuestEventType::KillEnemy, EnemyId, DeltaCount); }
void UGuideQuestSubsystem::ReportKillBoss(FName BossId) { ReportProgress(EGuideQuestEventType::KillBoss, BossId, 1); }

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
	if (!ActiveQuestRow)
	{
		return;
	}

	const FName CompletedQuestId = ActiveQuestId;

	if (!CompletedQuestIds.Contains(CompletedQuestId))
	{
		CompletedQuestIds.Add(CompletedQuestId);
	}

	// [수정] 보상 코드는 여기에 한 번만 존재해야 합니다!
	const bool bRewardSuccess = GiveReward(GetPlayerCharacter());
	if (!bRewardSuccess)
	{
		UE_LOG(LogGuideQuest, Warning, TEXT("[GuideQuest] Reward failed for quest %s"), *CompletedQuestId.ToString());
	}

	const FName NextQuestId = ActiveQuestRow->NextQuestId;

	ActiveQuestId = NAME_None;
	ActiveQuestRow = nullptr;
	ActiveObjectives.Reset();

	OnGuideQuestCompleted.Broadcast(CompletedQuestId);
	OnGuideQuestUpdated.Broadcast();

	if (!NextQuestId.IsNone())
	{
		StartQuest(NextQuestId, true);
	}
}

// [수정] 구현부가 누락되었던 GiveReward 함수 추가
bool UGuideQuestSubsystem::GiveReward(APlayerCharacter_SB* PlayerCharacter)
{
	if (!PlayerCharacter) return false;

	// TODO: 실제 아이템/골드 지급 로직 작성 (예전 DataAsset 방식에서 쓰셨던 로직 복구 필요)
	// 예: PlayerCharacter->ModifyGold(ActiveQuestRow->RewardGold); 등

	return true;
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
	if (!ActiveQuestRow) return FText::GetEmpty();

	switch (ActiveQuestRow->RewardType)
	{
	case EGuideQuestRewardType::Gold:
		return FText::FromString(FString::Printf(TEXT("보상: 골드 %d"), ActiveQuestRow->RewardGold));
	case EGuideQuestRewardType::Item:
		return FText::FromString(FString::Printf(TEXT("보상: 아이템 %s x%d"), *ActiveQuestRow->RewardItemId.ToString(), ActiveQuestRow->RewardItemCount));
	default:
		return FText::FromString(TEXT("보상: 없음"));
	}
}