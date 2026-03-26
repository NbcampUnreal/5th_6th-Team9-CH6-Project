#include "QuestComponent.h"
#include "Character/PlayerCharacter_SB.h"
#include "Inventory/InventoryComponent.h"

UQuestComponent::UQuestComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
} 

void UQuestComponent::BeginPlay()
{
    Super::BeginPlay();
}

bool UQuestComponent::AcceptQuest(int32 QuestID)
{
    // 이미 진행 중이거나 완료된 퀘스트 체크
    EQuestState CurrentState = GetQuestState(QuestID);
    if (CurrentState != EQuestState::None && CurrentState != EQuestState::Available)
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestComponent] Quest %d already accepted or completed"), QuestID);
        return false;
    }

    // DataTable에서 퀘스트 데이터 확인
    FQuestDataRow* QuestData = GetQuestData(QuestID);
    if (!QuestData)
    {
        UE_LOG(LogTemp, Error, TEXT("[QuestComponent] Quest %d not found in DataTable"), QuestID);
        return false;
    }

    // 진행 목록에 추가
    FQuestProgress NewProgress;
    NewProgress.QuestID = QuestID;
    NewProgress.State = EQuestState::Active;
    NewProgress.CurrentCount = 0;

    int32 ExistingIndex = FindQuestProgressIndex(QuestID);
    if (ExistingIndex != INDEX_NONE)
    {
        QuestProgressList[ExistingIndex] = NewProgress;
    }
    else
    {
        QuestProgressList.Add(NewProgress);
    }

    OnQuestUpdated.Broadcast(QuestID, EQuestState::Active);

    UE_LOG(LogTemp, Log, TEXT("[QuestComponent] Quest accepted: %d - %s"),
        QuestID, *QuestData->QuestName.ToString());

    return true;
}

bool UQuestComponent::CompleteQuest(int32 QuestID)
{
    if (!IsQuestCompletable(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestComponent] Quest %d is not completable"), QuestID);
        return false;
    }

    FQuestDataRow* QuestData = GetQuestData(QuestID);
    if (!QuestData) return false;

    // 보상 지급
    APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(GetOwner());
    if (Player && QuestData->RewardGold > 0)
    {
        UInventoryComponent* Inventory = Player->GetInventory();
        if (Inventory)
        {
            UItemBase* GoldItem = Inventory->CreateItemInstanceByID(
                FName(TEXT("700001")), QuestData->RewardGold);
            if (GoldItem)
            {
                Inventory->HandleAddItem_AutoHotbarFirst(GoldItem);
                UE_LOG(LogTemp, Log, TEXT("[QuestComponent] Quest reward: %dG"),
                    QuestData->RewardGold);
            }
        }
    }

    // 퀘스트 아이템 차감
    if (Player && QuestData->QuestType == EQuestType::Collect
        && !QuestData->TargetItemID.IsNone()
        && QuestData->TargetCount > 0)
    {
        UInventoryComponent* Inventory = Player->GetInventory();
        if (Inventory)
        {
            Inventory->ConsumeByID(QuestData->TargetItemID, QuestData->TargetCount);
            UE_LOG(LogTemp, Log, TEXT("[QuestComponent] Consumed %dx %s"),
                QuestData->TargetCount, *QuestData->TargetItemID.ToString());
        }
    }

    // 상태 변경
    int32 Index = FindQuestProgressIndex(QuestID);
    if (Index != INDEX_NONE)
    {
        QuestProgressList[Index].State = EQuestState::Rewarded;
    }

    OnQuestUpdated.Broadcast(QuestID, EQuestState::Rewarded);

    UE_LOG(LogTemp, Log, TEXT("[QuestComponent] Quest completed: %d"), QuestID);

    return true;
}

void UQuestComponent::OnItemCollected(FName ItemID, int32 Amount)
{
    for (FQuestProgress& Progress : QuestProgressList)
    {
        if (Progress.State != EQuestState::Active) continue;

        FQuestDataRow* QuestData = GetQuestData(Progress.QuestID);
        if (!QuestData) continue;

        // 수집 퀘스트이고 목표 아이템이 일치하는지 확인
        if (QuestData->QuestType == EQuestType::Collect &&
            QuestData->TargetItemID == ItemID)
        {
            Progress.CurrentCount = FMath::Min(
                Progress.CurrentCount + Amount,
                QuestData->TargetCount
            );

            OnQuestProgressUpdated.Broadcast(Progress.QuestID, Progress.CurrentCount);

            UE_LOG(LogTemp, Log, TEXT("[QuestComponent] Quest %d progress: %d/%d"),
                Progress.QuestID, Progress.CurrentCount, QuestData->TargetCount);

            // 목표 달성 시 완료 가능 상태로 변경
            if (Progress.CurrentCount >= QuestData->TargetCount)
            {
                Progress.State = EQuestState::Completed;
                OnQuestUpdated.Broadcast(Progress.QuestID, EQuestState::Completed);

                UE_LOG(LogTemp, Log, TEXT("[QuestComponent] Quest %d ready to complete!"),
                    Progress.QuestID);
            }
        }
    }
}

EQuestState UQuestComponent::GetQuestState(int32 QuestID) const
{
    int32 Index = FindQuestProgressIndex(QuestID);
    if (Index == INDEX_NONE) return EQuestState::None;
    return QuestProgressList[Index].State;
}

FQuestProgress UQuestComponent::GetQuestProgress(int32 QuestID) const
{
    int32 Index = FindQuestProgressIndex(QuestID);
    if (Index == INDEX_NONE) return FQuestProgress();
    return QuestProgressList[Index];
}

TArray<FQuestProgress> UQuestComponent::GetActiveQuests() const
{
    TArray<FQuestProgress> ActiveQuests;
    for (const FQuestProgress& Progress : QuestProgressList)
    {
        if (Progress.State == EQuestState::Active ||
            Progress.State == EQuestState::Completed)
        {
            ActiveQuests.Add(Progress);
        }
    }
    return ActiveQuests;
}

FQuestDataRow* UQuestComponent::GetQuestData(int32 QuestID) const
{
    if (!QuestDataTable) return nullptr;

    TArray<FName> RowNames = QuestDataTable->GetRowNames();
    for (FName RowName : RowNames)
    {
        FQuestDataRow* Row = QuestDataTable->FindRow<FQuestDataRow>(RowName, TEXT("GetQuestData"));
        if (Row && Row->QuestID == QuestID)
        {
            return Row;
        }
    }
    return nullptr;
}

bool UQuestComponent::IsQuestCompletable(int32 QuestID) const
{
    EQuestState State = GetQuestState(QuestID);
    return State == EQuestState::Completed;
}

int32 UQuestComponent::FindQuestProgressIndex(int32 QuestID) const
{
    for (int32 i = 0; i < QuestProgressList.Num(); ++i)
    {
        if (QuestProgressList[i].QuestID == QuestID)
            return i;
    }
    return INDEX_NONE;
}