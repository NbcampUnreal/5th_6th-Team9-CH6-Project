#include "Player/SBTitlePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Settings/SBGameUserSettings.h"

void ASBTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	if (IsLocalController())
	{
		if (USBGameUserSettings* Settings = USBGameUserSettings::Get())
		{
			Settings->LoadSettings(false);
			Settings->ApplyAudioSettings(this);
		}
	}

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