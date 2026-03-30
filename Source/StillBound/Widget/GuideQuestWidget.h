#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GuideQuestWidget.generated.h"

class UTextBlock;
class UGuideQuestSubsystem;

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