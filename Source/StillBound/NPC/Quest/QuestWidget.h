#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestData.h"
#include "QuestWidget.generated.h"

class UTextBlock;
class UVerticalBox;
class UQuestComponent;
class UQuestObjectiveSlot;

UCLASS()
class STILLBOUND_API UQuestWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

public:
    // 퀘스트 목표 업데이트
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RefreshQuestObjectives();

    // QuestComponent 설정
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SetQuestComponent(UQuestComponent* InQuestComponent);

    // 퀘스트 상태 변경 콜백
    UFUNCTION()
    void OnQuestUpdated(int32 QuestID, EQuestState NewState);

    // 퀘스트 진행도 변경 콜백
    UFUNCTION()
    void OnQuestProgressUpdated(int32 QuestID, int32 CurrentCount);

protected:
    // 퀘스트 목록을 담는 VerticalBox (블루프린트에서 바인딩)
    UPROPERTY(meta = (BindWidget))
    UVerticalBox* VB_QuestObjectives;

    // 퀘스트 없을 때 표시 텍스트
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_NoQuest;

    // 개별 퀘스트 항목 위젯 클래스
    UPROPERTY(EditDefaultsOnly, Category = "Quest")
    TSubclassOf<UQuestObjectiveSlot> QuestObjectiveSlotClass;

private:
    UPROPERTY()
    UQuestComponent* QuestComponent;
};