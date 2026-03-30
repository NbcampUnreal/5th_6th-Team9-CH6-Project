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

    // 상태 변경
    int32 Index = FindQuestProgressIndex(QuestID);
    if (Index != INDEX_NONE)
    {
        QuestProgressList[Index].State = EQuestState::Rewarded;
    }
    OnQuestUpdated.Broadcast(QuestID, EQuestState::Rewarded);

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
    UE_LOG(LogTemp, Log, TEXT("[QuestComponent] Quest completed: %d"), QuestID);

    return true;
}

void UQuestComponent::OnItemCollected(FName ItemID, int32 Amount)
{
    SyncCollectQuestProgress(ItemID);
}

void UQuestComponent::OnItemRemoved(FName ItemID, int32 Amount)
{
    SyncCollectQuestProgress(ItemID);
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
    if (State != EQuestState::Completed) return false;

    FQuestDataRow* QuestData = GetQuestData(QuestID);
    if (!QuestData) return false;

    if (QuestData->QuestType == EQuestType::Collect && !QuestData->TargetItemID.IsNone())
    {
        APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(GetOwner());
        if (!Player) return false;

        UInventoryComponent* Inventory = Player->GetInventory();
        if (!Inventory) return false;

        int32 CurrentCount = Inventory->GetTotalCountByID(QuestData->TargetItemID);
        if (CurrentCount < QuestData->TargetCount)
        {
            return false;
        }
    }
    return true;
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

void UQuestComponent::SyncCollectQuestProgress(FName ItemID)
{
    APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(GetOwner());
    if (!Player) return;

    UInventoryComponent* Inventory = Player->GetInventory();
    if (!Inventory) return;

    for (FQuestProgress& Progress : QuestProgressList)
    {
        // Active(진행 중)과 Completed(보고 대기) 상태만 검사
        // 이미 보상을 받은 상태면 건너뜀
        if (Progress.State != EQuestState::Active && Progress.State != EQuestState::Completed)
        {
            continue;
        }
        FQuestDataRow* QuestData = GetQuestData(Progress.QuestID);
        if (!QuestData) continue;

        // 수집 퀘스트이고, 대상 아이템이 일치할 경우에만 로직 수행
        if (QuestData->QuestType == EQuestType::Collect && QuestData->TargetItemID == ItemID)
        {
            int32 OldCount = Progress.CurrentCount;
            EQuestState OldState = Progress.State;

            // 인벤토리 기반 수량 동기화 (최대 목표치를 넘지 않도록)
            Progress.CurrentCount = FMath::Min(Inventory->GetTotalCountByID(ItemID), QuestData->TargetCount);

            // 값이 실제로 변경됐을 때만 UI 업뎃 방송
            if (OldCount != Progress.CurrentCount)
            {
                OnQuestProgressUpdated.Broadcast(Progress.QuestID, Progress.CurrentCount);
            }
            // 상태 전이 체크
            if (Progress.CurrentCount >= QuestData->TargetCount && OldState == EQuestState::Active)
            {
                // 조건 달성: 진행중->완료 대기
                Progress.State = EQuestState::Completed;
                OnQuestUpdated.Broadcast(Progress.QuestID, EQuestState::Completed);
            }
            else if (Progress.CurrentCount < QuestData->TargetCount && OldState == EQuestState::Completed)
            {
                // 아이템 상실로 인한 롤백(완료 대기->진행 중)
                Progress.State = EQuestState::Active;
                OnQuestUpdated.Broadcast(Progress.QuestID, EQuestState::Active);
            }
        }
    }
}
