
#include "Character/PlayerController_SB.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"

void APlayerController_SB::SetupInputComponent()
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
	EnhancedInputComponent->BindAction(EvasionAction, ETriggerEvent::Triggered, this, &ThisClass::Evasion);
	EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ThisClass::Interact);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ThisClass::Attack);
	EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Triggered, this, &ThisClass::Skill);
	EnhancedInputComponent->BindAction(Hotbar1Action, ETriggerEvent::Triggered, this, &ThisClass::SelectHotbar1);
	EnhancedInputComponent->BindAction(Hotbar2Action, ETriggerEvent::Triggered, this, &ThisClass::SelectHotbar2);
	EnhancedInputComponent->BindAction(Hotbar3Action, ETriggerEvent::Triggered, this, &ThisClass::SelectHotbar3);
	EnhancedInputComponent->BindAction(Hotbar4Action, ETriggerEvent::Triggered, this, &ThisClass::SelectHotbar4);
}


#pragma region ========================= Input - Movement =========================

void APlayerController_SB::Move(const FInputActionValue& Value)
{
	if (!IsValid(GetPawn())) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	GetPawn()->AddMovementInput(ForwardDirection, MovementVector.Y);
	GetPawn()->AddMovementInput(RightDirection, MovementVector.X);
}

void APlayerController_SB::Look(const FInputActionValue& Value)
{
	if (!IsValid(GetPawn())) return;

	const FVector2D LookVector = Value.Get<FVector2D>();

	AddYawInput(LookVector.X);
	AddPitchInput(LookVector.Y);
}

void APlayerController_SB::Jump()
{
	if (!IsValid(GetCharacter())) return;

	GetCharacter()->Jump();
}

void APlayerController_SB::StopJumping()
{
	if (!IsValid(GetCharacter())) return;

	GetCharacter()->StopJumping();
}

void APlayerController_SB::ToggleCrouch()
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

void APlayerController_SB::Interact()
{
}

#pragma endregion


#pragma region ========================= Input - Abilities =========================

void APlayerController_SB::ActivateAbility(const FGameplayTag& AbilityTag) const
{
	APawn* P = GetPawn();
	if (!P) { return; }

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(P);
	if (!ASI) { return; }

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) { return; }

	FGameplayTagContainer AbilityTags;
	AbilityTags.AddTag(AbilityTag);

	const bool bActivated = ASC->TryActivateAbilitiesByTag(AbilityTags);

	UE_LOG(LogTemp, Log, TEXT("[PC] ActivateAbility(%s) -> %d"),
		*AbilityTag.ToString(), bActivated);
}

void APlayerController_SB::Evasion()
{
	const FGameplayTag EvasionTag = FGameplayTag::RequestGameplayTag(TEXT("Player.Ability.Evasion"));
	ActivateAbility(EvasionTag);
}

void APlayerController_SB::Attack()
{
	const FGameplayTag EvasionTag = FGameplayTag::RequestGameplayTag(TEXT("Player.Ability.Attack"));
	ActivateAbility(EvasionTag);
}

void APlayerController_SB::Skill()
{
}

#pragma endregion


#pragma region ========================= Input - Hotbar =========================

void APlayerController_SB::SelectHotbar1()
{
}

void APlayerController_SB::SelectHotbar2()
{
}

void APlayerController_SB::SelectHotbar3()
{
}

void APlayerController_SB::SelectHotbar4()
{
}

#pragma endregion