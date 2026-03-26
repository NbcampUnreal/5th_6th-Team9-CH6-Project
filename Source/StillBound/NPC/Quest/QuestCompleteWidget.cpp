// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Quest/QuestCompleteWidget.h"
#include "NPC/NPCCharacter_Quest.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "EngineUtils.h"

void UQuestCompleteWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BTN_Confirm)
	{
		BTN_Confirm->OnClicked.AddUniqueDynamic(this, &UQuestCompleteWidget::OnConfirmClicked);
	}
}

void UQuestCompleteWidget::InitializePopup(const FText& QuestName, int32 RewardGold)
{
	if (TXT_QuestName)
	{
		TXT_QuestName->SetText(QuestName);
	}
	if (TXT_RewardGold)
	{
		TXT_RewardGold->SetText(FText::Format(
			FText::FromString(TEXT("Reward: {0}G")),
			FText::AsNumber(RewardGold)
		));
	}
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void UQuestCompleteWidget::OnConfirmClicked()
{
 	// NPCCharacter_Quest의 포인터 초기화
    // 월드에서 QuestNPC 찾아서 ActiveQuestCompleteWidget 클리어
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ANPCCharacter_Quest> It(World); It; ++It)
		{
			if (It->ActiveQuestCompleteWidget == this)
			{
				It->ActiveQuestCompleteWidget = nullptr;
				break;
			}
		}
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
	RemoveFromParent();
}
