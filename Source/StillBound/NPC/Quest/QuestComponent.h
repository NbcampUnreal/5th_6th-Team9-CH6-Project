#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestData.h"
#include "QuestComponent.generated.h"

// 퀘스트 상태 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestUpdated, int32, QuestID, EQuestState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestProgressUpdated, int32, QuestID, int32, CurrentCount);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class STILLBOUND_API UQuestComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestComponent();

protected:
    virtual void BeginPlay() override;

public:
    // 퀘스트 DataTable
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    UDataTable* QuestDataTable;

    // 퀘스트 상태 변경 이벤트
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnQuestUpdated OnQuestUpdated;

    // 퀘스트 진행도 변경 이벤트
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnQuestProgressUpdated OnQuestProgressUpdated;

    // 퀘스트 수락
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AcceptQuest(int32 QuestID);

    // 퀘스트 완료 처리 (NPC에게 완료 보고)
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(int32 QuestID);

    // 아이템 수집 시 진행도 업데이트
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnItemCollected(FName ItemID, int32 Amount);

    // 인벤토리에서 아이템이 소모/버려졌을 때 호출 (델리게이트 등에 바인딩)
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnItemRemoved(FName ItemID, int32 Amount);

    // 퀘스트 상태 조회
    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuestState GetQuestState(int32 QuestID) const;

    // 퀘스트 진행도 조회
    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuestProgress GetQuestProgress(int32 QuestID) const;

    // 현재 활성 퀘스트 목록 조회 (목표 UI용)
    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuestProgress> GetActiveQuests() const;

    // DataTable에서 퀘스트 데이터 조회
    FQuestDataRow* GetQuestData(int32 QuestID) const;

    // 퀘스트 완료 가능 여부
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestCompletable(int32 QuestID) const;

private:
    // 런타임 퀘스트 진행 목록
    UPROPERTY()
    TArray<FQuestProgress> QuestProgressList;

    // 진행 목록에서 인덱스 찾기
    int32 FindQuestProgressIndex(int32 QuestID) const;

    // 실제 퀘스트 진척도와 상태를 인벤토리와 동기화하는 핵심 함수
    void SyncCollectQuestProgress(FName ItemID);
};