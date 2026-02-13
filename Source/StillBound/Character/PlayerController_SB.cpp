
#include "Character/PlayerController_SB.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "Character/BaseCharacter_SB.h"
#include "Character/PlayerCharacter_SB.h"
#include "Character/PlayerAttributeSet.h"
#include "UI/USB_UIManager.h"
#include "UI/UW_UIHUD.h"
#include "UI/UW_Minimap.h"
#include "UI/Map/MapWorldManager.h"
#include "UI/UW_FullMap.h"
#include "Landscape.h"
#include "EngineUtils.h"
#include "EngineUtils.h"
#include "Weapons/WeaponBase.h"
#include "Subsystem/SBWorldSaveManagerSubsystem.h"

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
	EnhancedInputComponent->BindAction(EmoteAction, ETriggerEvent::Started, this, &ThisClass::Emote);
	EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ThisClass::BeginInteract);
	EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ThisClass::EndInteract);

	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::Attack);
	EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Triggered, this, &ThisClass::Skill);
	EnhancedInputComponent->BindAction(Hotbar1Action, ETriggerEvent::Triggered, this, &ThisClass::SelectHotbar1);
	EnhancedInputComponent->BindAction(Hotbar2Action, ETriggerEvent::Triggered, this, &ThisClass::SelectHotbar2);
	EnhancedInputComponent->BindAction(Hotbar3Action, ETriggerEvent::Triggered, this, &ThisClass::SelectHotbar3);
	EnhancedInputComponent->BindAction(Hotbar4Action, ETriggerEvent::Triggered, this, &ThisClass::SelectHotbar4);
	EnhancedInputComponent->BindAction(ToggleMenuAction, ETriggerEvent::Started, this, &ThisClass::ToggleMenu);
	EnhancedInputComponent->BindAction(FullMapAction,ETriggerEvent::Started,this,&ThisClass::ToggleFullMap);
}

#pragma region ========================= Input - Movement =========================


void APlayerController_SB::Move(const FInputActionValue& Value)
{
	if (!IsValid(GetPawn())) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (!MovementVector.IsNearlyZero())
	{
		CancelEmoteAbility();
	}

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
	ACharacter* Char = GetCharacter();
	if (!IsValid(Char)) return;

	
	if (!Char->CanJump())
	{
		return;
	}

	CancelEmoteAbility();

	Char->Jump();
}

void APlayerController_SB::StopJumping()
{
	if (!IsValid(GetCharacter())) return;

	GetCharacter()->StopJumping();
}

void APlayerController_SB::ToggleCrouch()
{
	ACharacter* Char = GetCharacter();
	if (!IsValid(Char)) return;

	if (UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement())
	{
		if (MoveComp->IsFalling())
		{
			return;
		}
	}

	UAbilitySystemComponent* ASC =UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Char);
	if (ASC)
	{
		const FGameplayTag NoCrouchTag = FGameplayTag::RequestGameplayTag(TEXT("State.Action"));
		if (ASC->HasMatchingGameplayTag(NoCrouchTag))
		{
			return;
		}
	}

	if (Char->bIsCrouched)
	{
		Char->UnCrouch();
	}
	else
	{
		Char->Crouch();
	}
}

void APlayerController_SB::BeginInteract()
{
	if (APlayerCharacter_SB* PlayerChar = Cast<APlayerCharacter_SB>(GetPawn()))
	{
		PlayerChar->BeginInteract();
	}
}

void APlayerController_SB::EndInteract()
{
	if (auto* PC = Cast<APlayerCharacter_SB>(GetPawn()))
	{
		// [핵심] 만약 지금 '대화 중'이거나 '잠금 상태'라면, 
		// 키를 뗐을 때 발생하는 종료 신호를 여기서 씹어버립니다(return).
		if (PC->IsInteracting())
		{
			return;
		}

		PC->EndInteract();
	}
}

void APlayerController_SB::ToggleMenu()
{
	UIManager->ToggleMenu();
}

#pragma endregion

#pragma region ========================= Input - Abilities =========================

bool APlayerController_SB::ActivateAbility(const FGameplayTag& AbilityTag) const
{
	APawn* P = GetPawn();
	if (!P) return false;

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(P);
	if (!ASI) return false;

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return false;

	FGameplayTagContainer AbilityTags;
	AbilityTags.AddTag(AbilityTag);

	const bool bActivated = ASC->TryActivateAbilitiesByTag(AbilityTags);

	UE_LOG(LogTemp, Log, TEXT("[PC] ActivateAbility(%s) -> %d"),
		*AbilityTag.ToString(), bActivated);
	return bActivated;
}


void APlayerController_SB::Evasion()
{
	ACharacter* Char = GetCharacter();
	if (!IsValid(Char)) return;

	if (Char->bIsCrouched)
	{
		Char->UnCrouch();
	}

	const FGameplayTag EvasionTag = FGameplayTag::RequestGameplayTag(TEXT("Player.Ability.Evasion"));

	ActivateAbility(EvasionTag);
}

void APlayerController_SB::Emote()
{
	const FGameplayTag EvasionTag = FGameplayTag::RequestGameplayTag(TEXT("Player.Ability.Emote"));

	ActivateAbility(EvasionTag);
}

void APlayerController_SB::CancelEmoteAbility()
{
	APawn* P = GetPawn();
	if (!IsValid(P)) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(P);
	if (!ASC) return;

	const FGameplayTag EmoteAbilityTag = FGameplayTag::RequestGameplayTag(TEXT("Player.Ability.Emote"));

	FGameplayTagContainer EmoteTags;
	EmoteTags.AddTag(EmoteAbilityTag);

	ASC->CancelAbilities(&EmoteTags);
}

bool APlayerController_SB::ActivateAbilityAttack(const FGameplayTag& InputTag) const
{
	// 1) Pawn -> PlayerCharacter
	const APlayerCharacter_SB* PC = Cast<APlayerCharacter_SB>(GetPawn());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] ActivateAbilityAttack: Pawn is not PlayerCharacter"));
		return false;
	}

	// 2) EquippedWeapon 가져오기
	AWeaponBase* Weapon = PC->GetEquippedWeapon();
	if (!Weapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] ActivateAbilityAttack: No EquippedWeapon"));
		return false;
	}

	// 3) 무기 내부 매핑(InputTag -> SpecHandle)로 GA 발동
	const bool bActivated = Weapon->ActivateByInputTag(InputTag);

	UE_LOG(LogTemp, Log, TEXT("[PC] ActivateAbilityAttack(%s) Weapon=%s -> %d"),
		*InputTag.ToString(),
		*GetNameSafe(Weapon),
		bActivated);

	return bActivated;

}

void APlayerController_SB::Attack()
{

	ACharacter* Char = GetCharacter();
	if (!IsValid(Char)) return;

	if (Char->bIsCrouched)
	{
		Char->UnCrouch();
	}

	//const FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(TEXT("Player.Ability.Attack"));
	//ActivateAbility(AttackTag);

	 // 무기 기본 공격 입력 태그로 발동
	const FGameplayTag InputAttackPrimary =
		FGameplayTag::RequestGameplayTag(TEXT("InputTag.Attack.Primary"), /*ErrorIfNotFound*/ false);

	if (!InputAttackPrimary.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] Attack: InputTag.Attack.Primary is not registered"));
		return;
	}

	ActivateAbilityAttack(InputAttackPrimary);
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

#pragma region ========================= UI =========================
void APlayerController_SB::BeginPlay()
{
	Super::BeginPlay();


	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;

	FInputModeGameOnly Mode;
	SetInputMode(Mode);

	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);

	if (UIManagerClass)
	{
		UIManager = NewObject<USB_UIManager>(this, UIManagerClass);
		if (UIManager)
		{
			UIManager->Init(this);
		}
	}

	// MapWorldManager 찾기 (월드에 배치된 것 1개)
	for (TActorIterator<AMapWorldManager> It(GetWorld()); It; ++It)
	{
		MapWorldManager = *It;
		break;
	}

	if (!MapWorldManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[Minimap] MapWorldManager NOT FOUND"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Minimap] MapWorldManager FOUND: %s"), *MapWorldManager->GetName());
	}

	for (TActorIterator<ALandscape> It(GetWorld()); It; ++It)
	{
		ALandscape* Landscape = *It;

		FBox Bounds = Landscape->GetComponentsBoundingBox(true);

		UE_LOG(LogTemp, Warning, TEXT("Landscape Bounds Min: %s"), *Bounds.Min.ToString());
		UE_LOG(LogTemp, Warning, TEXT("Landscape Bounds Max: %s"), *Bounds.Max.ToString());

		break;
	}

	if (auto* Sub = GetGameInstance() ? GetGameInstance()->GetSubsystem<USBWorldSaveManagerSubsystem>() : nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Gameplay] CurrentSlotId = %s"), *Sub->GetCurrentSlotId());
		Sub->TouchCurrentWorldLastPlayed();
	}
}

void APlayerController_SB::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABaseCharacter_SB* Char = Cast<ABaseCharacter_SB>(InPawn);
	if (!Char) return;

	UPlayerAttributeSet* AS = Char->GetPlayerAttributeSet();
	if (!AS) return;

	AS->OnHealthChanged.AddDynamic(this, &ThisClass::OnHealthChanged);
	AS->OnStaminaChanged.AddDynamic(this, &ThisClass::OnStaminaChanged);

	if (auto* Sub = GetGameInstance() ? GetGameInstance()->GetSubsystem<USBWorldSaveManagerSubsystem>() : nullptr)
	{
		const bool bOk = Sub->LoadCurrentWorldTransformToPawn(InPawn);
		UE_LOG(LogTemp, Warning, TEXT("[Gameplay] LoadCurrentWorldTransformToPawn -> %d"), bOk);
	}
}

APlayerController_SB::APlayerController_SB()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void APlayerController_SB::OnHealthChanged(float OldValue, float NewValue)
{
	if (!UIManager)
	{
		return;
	}

	UIManager->UpdateHUD();
}

void APlayerController_SB::OnStaminaChanged(float OldValue, float NewValue)
{
	if (!UIManager) return;
	UIManager->UpdateHUD();
}
#pragma endregion 

#pragma region ===== World Save =====

void APlayerController_SB::SB_SaveWorld()
{
	if (auto* Sub = GetGameInstance() ? GetGameInstance()->GetSubsystem<USBWorldSaveManagerSubsystem>() : nullptr)
	{
		const bool bOk = Sub->SaveCurrentWorldFromPawn(GetPawn());
		UE_LOG(LogTemp, Warning, TEXT("[Gameplay] SB_SaveWorld -> %d"), bOk);
	}
}

void APlayerController_SB::SB_LoadWorld()
{
	if (auto* Sub = GetGameInstance() ? GetGameInstance()->GetSubsystem<USBWorldSaveManagerSubsystem>() : nullptr)
	{
		const bool bOk = Sub->LoadCurrentWorldToPawn(GetPawn());
		UE_LOG(LogTemp, Warning, TEXT("[Gameplay] SB_LoadWorld -> %d"), bOk);
	}
}

void APlayerController_SB::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	APawn* P = GetPawn();

	if (P)
	{
		if (auto* Sub = GetGameInstance() ? GetGameInstance()->GetSubsystem<USBWorldSaveManagerSubsystem>() : nullptr)
		{
			const bool bSaved = Sub->SaveCurrentWorldFromPawn(P);
			Sub->TouchCurrentWorldLastPlayed();
			UE_LOG(LogTemp, Warning, TEXT("[Gameplay] AutoSave(EndPlay) -> %d"), bSaved);
		}
	}

	Super::EndPlay(EndPlayReason);
}
#pragma endregion

void APlayerController_SB::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocalController()) return;

	if (!MapWorldManager || !UIManager) return;

	APawn* ControlledPawn = GetPawn();

	if (!ControlledPawn) return;
	const FVector WorldLoc = ControlledPawn->GetActorLocation();
	const FVector2D UV = MapWorldManager->WorldToUV(WorldLoc);
	if (UUW_UIHUD* HUD = UIManager->GetHUD())
	{
		if (UUW_Minimap* MinimapWidget = HUD->GetMiniMapWidget())
		{
			FVector2D PlayerUV = UV;
			PlayerUV.X = FMath::Clamp(PlayerUV.X, 0.f, 1.f);
			PlayerUV.Y = FMath::Clamp(PlayerUV.Y, 0.f, 1.f);
			MinimapWidget->UpdateMapOffset(PlayerUV);
			const float Yaw = ControlledPawn->GetActorRotation().Yaw;
			MinimapWidget->UpdatePlayerIconRotation(Yaw);
		}
	}

	if (UUW_FullMap* FullMap = UIManager->GetFullMapWidget())
	{
		if (FullMap->IsInViewport())
		{
			FVector2D PlayerUV = UV;
			PlayerUV.X = FMath::Clamp(PlayerUV.X, 0.f, 1.f);
			PlayerUV.Y = FMath::Clamp(PlayerUV.Y, 0.f, 1.f);
			FullMap->UpdatePlayerPosition(PlayerUV);
		}
	}
}

void APlayerController_SB::ToggleFullMap()
{
	UE_LOG(LogTemp, Warning, TEXT("FullMap Key Pressed"));
	if (UIManager)
	{
		UIManager->ToggleFullMap();
	}
}

#pragma endregion
