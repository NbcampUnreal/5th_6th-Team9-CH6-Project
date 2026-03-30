#include "UI/UW_GameClear.h"
#include "Components/TextBlock.h"
#include "Character/PlayerController_SB.h"

void UUW_GameClear::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUW_GameClear::SetGameClear(bool bBossKilled)
{
	if (!ResultText) return;

	if (bBossKilled)
	{
		ResultText->SetText(FText::FromString("GAME CLEAR"));

		if (GameClearMove)
		{
			PlayAnimation(GameClearMove);
		}

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			this,
			&UUW_GameClear::RemoveFromParent,
			10.0f,
			false
		);
	}
}

//클리어 위젯 숨기는 조건 추가
void USB_UIManager::HideGameClear()
{
	if (GameClearWidget)
	{
		GameClearWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

