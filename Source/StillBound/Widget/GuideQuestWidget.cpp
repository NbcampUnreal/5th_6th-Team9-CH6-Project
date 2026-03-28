#include "GuideQuestWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "GuideQuest/GuideQuestSubsystem.h"

void UGuideQuestWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UGuideQuestSubsystem* Subsystem = GI->GetSubsystem<UGuideQuestSubsystem>())
		{
			CachedSubsystem = Subsystem;
			Subsystem->OnGuideQuestUpdated.AddDynamic(this, &ThisClass::HandleGuideQuestUpdated);
			Subsystem->OnGuideQuestCompleted.AddDynamic(this, &ThisClass::HandleGuideQuestCompleted);
			Subsystem->OnGuideQuestRewardToast.AddDynamic(this, &ThisClass::HandleGuideQuestRewardToast);
		}
	}
	RefreshFromSubsystem();
}

void UGuideQuestWidget::NativeDestruct()
{
	if (CachedSubsystem.IsValid())
	{
		CachedSubsystem->OnGuideQuestUpdated.RemoveDynamic(this, &ThisClass::HandleGuideQuestUpdated);
		CachedSubsystem->OnGuideQuestCompleted.RemoveDynamic(this, &ThisClass::HandleGuideQuestCompleted);
		CachedSubsystem->OnGuideQuestRewardToast.RemoveDynamic(this, &ThisClass::HandleGuideQuestRewardToast);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RewardToastTimerHandle);
	}
	Super::NativeDestruct();
}

void UGuideQuestWidget::HandleGuideQuestUpdated() { RefreshFromSubsystem(); }
void UGuideQuestWidget::HandleGuideQuestCompleted(FName CompletedQuestId) { RefreshFromSubsystem(); }

void UGuideQuestWidget::RefreshFromSubsystem()
{
	const bool bHasActiveQuest = CachedSubsystem.IsValid() && CachedSubsystem->HasActiveQuest();

	//시스템이 유효하지 않고, 현재 진행중인 퀘스트도 없고, 보상 토스트도 없다면 위젯을 숨기기
	if (!bHasActiveQuest && !bShowingRewardToast)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 퀘스트 또는 보상 토스트 중 하나라도 있으면 위젯을 화면에 노출
	SetVisibility(ESlateVisibility::HitTestInvisible);
	if (TXT_GuideQuestTitle)
	{
		TXT_GuideQuestTitle->SetVisibility(bHasActiveQuest ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

		if (bHasActiveQuest)
		{
			TXT_GuideQuestTitle->SetText(CachedSubsystem->GetActiveQuestTitleText());
		}
	}

	if (TXT_GuideQuestObjectives)
	{
		TXT_GuideQuestObjectives->SetVisibility(bHasActiveQuest ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

		if (bHasActiveQuest)
		{
			TXT_GuideQuestObjectives->SetText(CachedSubsystem->BuildObjectiveProgressText());
		}
	}

	if (TXT_GuideQuestReward)
	{
		TXT_GuideQuestReward->SetVisibility(bHasActiveQuest ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

		if (bHasActiveQuest)
		{
			TXT_GuideQuestReward->SetText(CachedSubsystem->BuildRewardText());
		}
	}

	if (TXT_RewardToast && !bShowingRewardToast)
	{
		TXT_RewardToast->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UGuideQuestWidget::HandleGuideQuestRewardToast(FText RewardText) 
{
	bShowingRewardToast = true;

	if (TXT_RewardToast)
	{
		TXT_RewardToast->SetText(RewardText);
		TXT_RewardToast->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RewardToastTimerHandle);
		World->GetTimerManager().SetTimer(
			RewardToastTimerHandle,
			this,
			&ThisClass::HideRewardToast,
			2.0f,
			false
		);
	}
}

void UGuideQuestWidget::HideRewardToast()
{
	bShowingRewardToast = false;

	if (TXT_RewardToast)
	{
		TXT_RewardToast->SetVisibility(ESlateVisibility::Collapsed);
	}

	RefreshFromSubsystem();
}