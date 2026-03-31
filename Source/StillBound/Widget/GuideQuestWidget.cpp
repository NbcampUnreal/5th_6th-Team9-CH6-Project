#include "GuideQuestWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "GuideQuest/GuideQuestSubsystem.h"

void UGuideQuestWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 시작 시 토스트 박스는 무조건 숨김
	if (BD_RewardToast)
	{
		BD_RewardToast->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 디자이너 기본 문구 제거
	if (TXT_RewardToast)
	{
		TXT_RewardToast->SetText(FText::GetEmpty());
		TXT_RewardToast->SetVisibility(ESlateVisibility::Visible);
	}

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

	if (!bHasActiveQuest && !bShowingRewardToast)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

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

	// 텍스트가 아니라 박스 전체를 숨김
	if (BD_RewardToast && !bShowingRewardToast)
	{
		BD_RewardToast->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UGuideQuestWidget::HandleGuideQuestRewardToast(FText RewardText)
{
	bShowingRewardToast = true;

	float MaxToastWidth = 420.f;

	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize(0.f, 0.f);
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		MaxToastWidth = FMath::Max(280.f, ViewportSize.X * 0.35f);
	}

	if (SB_RewardToast)
	{
		SB_RewardToast->SetMaxDesiredWidth(MaxToastWidth);
	}

	if (TXT_RewardToast)
	{
		TXT_RewardToast->SetText(RewardText);
		TXT_RewardToast->SetAutoWrapText(true);
		TXT_RewardToast->SetWrapTextAt(MaxToastWidth - 40.f);
		TXT_RewardToast->SetVisibility(ESlateVisibility::Visible);
	}

	if (BD_RewardToast)
	{
		BD_RewardToast->SetVisibility(ESlateVisibility::HitTestInvisible);
		BD_RewardToast->ForceLayoutPrepass();
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

	if (BD_RewardToast)
	{
		BD_RewardToast->SetVisibility(ESlateVisibility::Collapsed);
	}

	RefreshFromSubsystem();
}