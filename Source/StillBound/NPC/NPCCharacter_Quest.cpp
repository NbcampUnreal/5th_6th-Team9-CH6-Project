#include "NPC/NPCCharacter_Quest.h"
#include "Quest/QuestComponent.h"
#include "Character/PlayerCharacter_SB.h"

ANPCCharacter_Quest::ANPCCharacter_Quest()
{
    NPCName = TEXT("Quest NPC");
}

void ANPCCharacter_Quest::BeginPlay()
{
    // ANPCCharacter::BeginPlay의 InitializeShopItems 스킵을 위해
    // AActor::BeginPlay()부터 직접 호출하는 대신
    // ItemDataTable을 임시로 null 처리
    UDataTable* TempTable = ItemDataTable;
    ItemDataTable = nullptr;  // 상점 초기화 스킵

    Super::BeginPlay();

    ItemDataTable = TempTable;  // 복구
}

bool ANPCCharacter_Quest::TryAcceptQuest(APlayerCharacter_SB* Player, int32 QuestID)
{
    if (!Player) return false;

    UQuestComponent* QuestComp = Player->FindComponentByClass<UQuestComponent>();
    if (!QuestComp)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] Player has no QuestComponent!"), *NPCName);
        return false;
    }

    bool bSuccess = QuestComp->AcceptQuest(QuestID);
    if (bSuccess)
    {
        OnQuestAccepted(QuestID);
    }
    return bSuccess;
}

bool ANPCCharacter_Quest::TryCompleteQuest(APlayerCharacter_SB* Player, int32 QuestID)
{
    if (!Player) return false;

    UQuestComponent* QuestComp = Player->FindComponentByClass<UQuestComponent>();
    if (!QuestComp)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] Player has no QuestComponent!"), *NPCName);
        return false;
    }

    bool bSuccess = QuestComp->CompleteQuest(QuestID);
    if (bSuccess)
    {
        OnQuestCompleted(QuestID);
    }
    return bSuccess;
}

int32 ANPCCharacter_Quest::GetAvailableQuestID(APlayerCharacter_SB* Player) const
{
    if (!Player) return 0;

    UQuestComponent* QuestComp = Player->FindComponentByClass<UQuestComponent>();
    if (!QuestComp) return 0;

    for (int32 QuestID : QuestIDs)
    {
        EQuestState State = QuestComp->GetQuestState(QuestID);
        
        // 이미 완료해서 보상을 받은 퀘스트일 경우, 다음 연계 퀘스트 확인
        if (State == EQuestState::Rewarded)
        {
            continue;
        }
        // 현재 진행 중이거나 보고 대기 중인 퀘스트--> 새로운 퀘스트 주지 않고 기다리기
        if (State == EQuestState::Active || State==EQuestState::Completed)
        {
            return 0;
        }
        // 아직 시작 안 한 퀘스트 발견 시
        if (State == EQuestState::None || State == EQuestState::Available)
        {
            return QuestID;
        }
    }
    return 0;
}

int32 ANPCCharacter_Quest::GetCompletableQuestID(APlayerCharacter_SB* Player) const
{
    if (!Player) return 0;

    UQuestComponent* QuestComp = Player->FindComponentByClass<UQuestComponent>();
    if (!QuestComp) return 0;

    for (int32 QuestID : QuestIDs)
    {
        if (QuestComp->IsQuestCompletable(QuestID))
        {
            return QuestID;
        }
    }
    return 0;
}

void ANPCCharacter_Quest::OnQuestAccepted_Implementation(int32 QuestID)
{
    UE_LOG(LogTemp, Log, TEXT("[%s] Quest accepted: %d"), *NPCName, QuestID);
}

void ANPCCharacter_Quest::OnQuestCompleted_Implementation(int32 QuestID)
{
    UE_LOG(LogTemp, Log, TEXT("[%s] Quest completed: %d"), *NPCName, QuestID);
}