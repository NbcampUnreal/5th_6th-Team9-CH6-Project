#include "Player/SBTitlePlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Settings/SBGameUserSettings.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "UserSettings/EnhancedInputUserSettings.h"

void ASBTitlePlayerController::RegisterTitleInputContexts()
{
	if (!IsLocalController()) return;

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!InputSubsystem) return;

	auto RegisterOne = [InputSubsystem](UInputMappingContext* IMC, int32 Priority)
		{
			if (!IMC) return;

			InputSubsystem->AddMappingContext(IMC, Priority);

			if (UEnhancedInputUserSettings* UserSettings = InputSubsystem->GetUserSettings())
			{
				UserSettings->RegisterInputMappingContext(IMC);
			}
		};

	RegisterOne(IMC_System, 10);
	RegisterOne(IMC_Movement, 0);
	RegisterOne(IMC_Abilities, 0);
	RegisterOne(IMC_Hotbar, 0);
}

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

	// 타이틀에서도 키 설정 row를 읽을 수 있게 등록
	RegisterTitleInputContexts();

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