#include "GuideQuestWidget.h"
#include "Components/TextBlock.h"
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
	}
	Super::NativeDestruct();
}

void UGuideQuestWidget::HandleGuideQuestUpdated() { RefreshFromSubsystem(); }
void UGuideQuestWidget::HandleGuideQuestCompleted(FName CompletedQuestId) { RefreshFromSubsystem(); }

void UGuideQuestWidget::RefreshFromSubsystem()
{
	//시스템이 유효하지 않거나, 현재 진행중인 퀘스트가 없다면 위젯을 숨기기
	if (!CachedSubsystem.IsValid() || !CachedSubsystem->HasActiveQuest())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 퀘스트가 있다면 위젯을 화면에 노출
	SetVisibility(ESlateVisibility::Visible);

	// Subsystem에 새로 만들어둔 Text Getter 함수들을 이용해 UI 적용
	if (TXT_GuideQuestTitle)
	{
		TXT_GuideQuestTitle->SetText(CachedSubsystem->GetActiveQuestTitleText());
	}

	if (TXT_GuideQuestObjectives)
	{
		TXT_GuideQuestObjectives->SetText(CachedSubsystem->BuildObjectiveProgressText());
	}

	if (TXT_GuideQuestReward)
	{
		TXT_GuideQuestReward->SetText(CachedSubsystem->BuildRewardText());
	}
}