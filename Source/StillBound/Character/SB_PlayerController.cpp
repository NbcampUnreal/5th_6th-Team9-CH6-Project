
#include "Character/SB_PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"


void ASB_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetLocalPlayer());
	if (!IsValid(InputSubsystem)) return;

	for (UInputMappingContext* Context : InputMappingContexts)
	{
		InputSubsystem->AddMappingContext(Context, 0);
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!IsValid(EnhancedInputComponent)) return;

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::StopJumping);
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::ToggleCrouch);
	EnhancedInputComponent->BindAction(EvasionAction, ETriggerEvent::Started, this, &ThisClass::Evasion);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ThisClass::Attack);
}


#pragma region ========================= Input - Movement =========================

void ASB_PlayerController::Move(const FInputActionValue& Value)
{
	if (!IsValid(GetPawn())) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	GetPawn()->AddMovementInput(ForwardDirection, MovementVector.Y);
	GetPawn()->AddMovementInput(RightDirection, MovementVector.X);
}

void ASB_PlayerController::Look(const FInputActionValue& Value)
{
	if (!IsValid(GetPawn())) return;

	const FVector2D LookVector = Value.Get<FVector2D>();

	AddYawInput(LookVector.X);
	AddPitchInput(LookVector.Y);
}

void ASB_PlayerController::Jump()
{
	if (!IsValid(GetCharacter())) return;

	GetCharacter()->Jump();
}

void ASB_PlayerController::StopJumping()
{
	if (!IsValid(GetCharacter())) return;

	GetCharacter()->StopJumping();
}

void ASB_PlayerController::ToggleCrouch()
{
	if (!IsValid(GetCharacter())) return;

	if (GetCharacter()->bIsCrouched)
	{
		GetCharacter()->UnCrouch();
	}
	else
	{
		GetCharacter()->Crouch();
	}
}

void ASB_PlayerController::Evasion()
{
}

#pragma endregion


#pragma region ========================= Input - Abilities =========================

void ASB_PlayerController::Attack()
{
}

void ASB_PlayerController::Skill()
{
}

#pragma endregion


#pragma region ========================= Input - Hotbar =========================

void ASB_PlayerController::SelectHotbar1()
{
}

void ASB_PlayerController::SelectHotbar2()
{
}

void ASB_PlayerController::SelectHotbar3()
{
}

void ASB_PlayerController::SelectHotbar4()
{
}

#pragma endregion