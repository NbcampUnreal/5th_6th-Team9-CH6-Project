#pragma once

#include "CoreMinimal.h"
#include "NPC/NPCCharacter.h"
#include "Quest/QuestData.h"
#include "Quest/QuestCompleteWidget.h"
#include "NPCCharacter_Quest.generated.h"

UCLASS()
class STILLBOUND_API ANPCCharacter_Quest : public ANPCCharacter
{
    GENERATED_BODY()

public:
    ANPCCharacter_Quest();

protected:
    virtual void BeginPlay() override;

public:
    // 이 NPC가 줄 수 있는 퀘스트 ID 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<int32> QuestIDs;

    // 퀘스트 수락 처리 (DialogueComponent에서 호출)
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool TryAcceptQuest(APlayerCharacter_SB* Player, int32 QuestID);

    // 퀘스트 완료 처리 (DialogueComponent에서 호출)
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool TryCompleteQuest(APlayerCharacter_SB* Player, int32 QuestID);

    // 플레이어에게 줄 수 있는 퀘스트 ID 반환 (진행 중이 아닌 것 중 첫 번째)
    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetAvailableQuestID(APlayerCharacter_SB* Player) const;

    // 플레이어가 완료 가능한 퀘스트 ID 반환
    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetCompletableQuestID(APlayerCharacter_SB* Player) const;

    // 블루프린트 이벤트
    UFUNCTION(BlueprintNativeEvent, Category = "Quest")
    void OnQuestAccepted(int32 QuestID);

    UFUNCTION(BlueprintNativeEvent, Category = "Quest")
    void OnQuestCompleted(int32 QuestID);

    UPROPERTY(EditDefaultsOnly, Category = "Quest")
    TSubclassOf<UQuestCompleteWidget> QuestCompleteWidgetClass;

    // 컴플릿 위젯 포인터
    UPROPERTY()
    UQuestCompleteWidget* ActiveQuestCompleteWidget;
};