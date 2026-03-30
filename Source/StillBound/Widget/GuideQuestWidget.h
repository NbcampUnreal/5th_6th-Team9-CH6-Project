#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GuideQuestWidget.generated.h"

class UTextBlock;
class UGuideQuestSubsystem;
class UBorder;
class USizeBox;

UCLASS()
class STILLBOUND_API UGuideQuestWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TXT_GuideQuestTitle;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TXT_GuideQuestObjectives;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TXT_GuideQuestReward;

	// 추가: 토스트 박스 루트
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> BD_RewardToast;

	// 추가: 토스트 최대 폭 제어용
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> SB_RewardToast;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TXT_RewardToast;

	UFUNCTION()
	void HandleGuideQuestUpdated();

	UFUNCTION()
	void HandleGuideQuestCompleted(FName CompletedQuestId);

	UFUNCTION()
	void HandleGuideQuestRewardToast(FText RewardText);

	void HideRewardToast();
	void RefreshFromSubsystem();

private:
	TWeakObjectPtr<UGuideQuestSubsystem> CachedSubsystem;
	FTimerHandle RewardToastTimerHandle;
	bool bShowingRewardToast = false;
};