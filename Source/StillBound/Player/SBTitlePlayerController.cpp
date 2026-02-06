#include "Player/SBTitlePlayerController.h"
#include "Blueprint/UserWidget.h"

void ASBTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	if (IsLocalController() && TitleRootWidgetClass)
	{
		TitleRootWidget = CreateWidget<UUserWidget>(this, TitleRootWidgetClass);
		if (TitleRootWidget)
		{
			TitleRootWidget->AddToViewport(0);

			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(TitleRootWidget->TakeWidget());
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(Mode);
		}
	}
}