#include "UI/UW_GameClear.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Character/PlayerController_SB.h"

void UUW_GameClear::NativeConstruct()
{
	Super::NativeConstruct();

	if (TitleButton)
	{
		TitleButton->OnClicked.AddDynamic(this, &UUW_GameClear::GoToTitleMenu);
	}
}

void UUW_GameClear::SetGameClear(bool bBossKilled)
{
	if (!ResultText) return;

	if (bBossKilled)
	{
		ResultText->SetText(FText::FromString("GAME CLEAR"));
	}
	else
	{
		ResultText->SetText(FText::FromString("YOU DIED"));
	}
}

void UUW_GameClear::GoToTitleMenu()
{
	APlayerController_SB* PC = Cast<APlayerController_SB>(GetOwningPlayer());

	if (PC)
	{
		PC->GoToTitleMenu();
	}
}
