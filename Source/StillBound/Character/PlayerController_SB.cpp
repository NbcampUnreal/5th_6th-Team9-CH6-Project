
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
#include "Weapons/WeaponBase.h"
#include "Subsystem/SBWorldSaveManagerSubsystem.h"
#include "Inventory/InventoryComponent.h"
#include "Build/BuildComponent.h"

void APlayerController_SB::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!IsValid(InputSubsystem)) return;

	if (IMC_Movement) InputSubsystem->AddMappingContext(IMC_Movement, 0);
	if (IMC_Abilities) InputSubsystem->AddMappingContext(IMC_Abilities, 0);
	if (IMC_Hotbar)   InputSubsystem->AddMappingContext(IMC_Hotbar, 0);

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

	EnhancedInputComponent->BindAction(MouseWheelAction, ETriggerEvent::Triggered, this, &ThisClass::OnMouseWheel);
	EnhancedInputComponent->BindAction(UseHotbarAction, ETriggerEvent::Started, this, &ThisClass::OnUseHotbar);
	EnhancedInputComponent->BindAction(HotbarSelectAction_1, ETriggerEvent::Started, this, &ThisClass::OnHotbar1);
	EnhancedInputComponent->BindAction(HotbarSelectAction_2, ETriggerEvent::Started, this, &ThisClass::OnHotbar2);
	EnhancedInputComponent->BindAction(HotbarSelectAction_3, ETriggerEvent::Started, this, &ThisClass::OnHotbar3);
	EnhancedInputComponent->BindAction(HotbarSelectAction_4, ETriggerEvent::Started, this, &ThisClass::OnHotbar4);
	EnhancedInputComponent->BindAction(HotbarSelectAction_5, ETriggerEvent::Started, this, &ThisClass::OnHotbar5);
	EnhancedInputComponent->BindAction(HotbarSelectAction_6, ETriggerEvent::Started, this, &ThisClass::OnHotbar6);
	EnhancedInputComponent->BindAction(HotbarSelectAction_7, ETriggerEvent::Started, this, &ThisClass::OnHotbar7);
	EnhancedInputComponent->BindAction(HotbarSelectAction_8, ETriggerEvent::Started, this, &ThisClass::OnHotbar8);
	EnhancedInputComponent->BindAction(HotbarSelectAction_9, ETriggerEvent::Started, this, &ThisClass::OnHotbar9);

	EnhancedInputComponent->BindAction(ToggleMenuAction, ETriggerEvent::Started, this, &ThisClass::ToggleMenu);
	EnhancedInputComponent->BindAction(FullMapAction,ETriggerEvent::Started,this,&ThisClass::ToggleFullMap);
	EnhancedInputComponent->BindAction(ToggleBuildAction, ETriggerEvent::Started, this, &ThisClass::ToggleBuild);
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
	if (IsGameplayInputBlocked()) return;

	if (!IsValid(GetCharacter())) return;

	GetCharacter()->StopJumping();
}

void APlayerController_SB::ToggleCrouch()
{
	if (IsGameplayInputBlocked()) return;

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
	if (IsGameplayInputBlocked()) return;

	if (APlayerCharacter_SB* PlayerChar = Cast<APlayerCharacter_SB>(GetPawn()))
	{
		PlayerChar->BeginInteract();
	}
}

void APlayerController_SB::EndInteract()
{
	if (IsGameplayInputBlocked()) return;

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
	if (!UIManager) return;

	if (bFullMapOpen)
	{
		UIManager->ToggleFullMap();
		bFullMapOpen = false;

		//ApplyOverlayInputState();
		return;
	}

	UIManager->ToggleMenu();
	bMenuOpen = !bMenuOpen;

	//ApplyOverlayInputState();
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
	if (IsGameplayInputBlocked()) return;

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
	if (IsGameplayInputBlocked()) return;

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
	if (IsGameplayInputBlocked()) return;

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
	if (IsGameplayInputBlocked()) return;
}

#pragma endregion

#pragma region ========================= Input - Hotbar =========================

void APlayerController_SB::OnMouseWheel(const FInputActionValue& Value)
{
	if (IsGameplayInputBlocked()) return;

	float Axis = Value.Get<float>();
	if (FMath::IsNearlyZero(Axis)) return;

	APlayerCharacter_SB* Char = Cast<APlayerCharacter_SB>(GetCharacter());
	Char->SelectHotbarIndex(Char->CurrentHotbarIndex + (Axis > 0.f ? -1 : 1));
}

void APlayerController_SB::OnHotbar1()
{
	if (IsGameplayInputBlocked()) return;

	APlayerCharacter_SB* Char = Cast<APlayerCharacter_SB>(GetCharacter());
	Char->SelectHotbarIndex(0);
}

void APlayerController_SB::OnHotbar2()
{
	if (IsGameplayInputBlocked()) return;

	APlayerCharacter_SB* Char = Cast<APlayerCharacter_SB>(GetCharacter());
	Char->SelectHotbarIndex(1);
}

void APlayerController_SB::OnHotbar3()
{
	if (IsGameplayInputBlocked()) return;

	APlayerCharacter_SB* Char = Cast<APlayerCharacter_SB>(GetCharacter());
	Char->SelectHotbarIndex(2);
}

void APlayerController_SB::OnHotbar4()
{
	if (IsGameplayInputBlocked()) return;

	APlayerCharacter_SB* Char = Cast<APlayerCharacter_SB>(GetCharacter());
	Char->SelectHotbarIndex(3);
}

void APlayerController_SB::OnHotbar5()
{
	if (IsGameplayInputBlocked()) return;

	APlayerCharacter_SB* Char = Cast<APlayerCharacter_SB>(GetCharacter());
	Char->SelectHotbarIndex(4);
}

void APlayerController_SB::OnHotbar6()
{
	if (IsGameplayInputBlocked()) return;

	APlayerCharacter_SB* Char = Cast<APlayerCharacter_SB>(GetCharacter());
	Char->SelectHotbarIndex(5);
}

void APlayerController_SB::OnHotbar7()
{
	if (IsGameplayInputBlocked()) return;

	APlayerCharacter_SB* Char = Cast<APlayerCharacter_SB>(GetCharacter());
	Char->SelectHotbarIndex(6);
}

void APlayerController_SB::OnHotbar8()
{
	if (IsGameplayInputBlocked()) return;

	APlayerCharacter_SB* Char = Cast<APlayerCharacter_SB>(GetCharacter());
	Char->SelectHotbarIndex(7);
}

void APlayerController_SB::OnHotbar9()
{
	if (IsGameplayInputBlocked()) return;

	APlayerCharacter_SB* Char = Cast<APlayerCharacter_SB>(GetCharacter());
	Char->SelectHotbarIndex(8);
}

void APlayerController_SB::OnUseHotbar(const FInputActionValue& Value)
{
	if (IsGameplayInputBlocked()) return;

	UE_LOG(LogTemp, Warning, TEXT("[UseHotbar] Started"));

	APlayerCharacter_SB* Char = Cast<APlayerCharacter_SB>(GetCharacter());

	Char->UseSelectedHotbarItem();
}

void APlayerController_SB::ToggleBuild()
{
	APlayerCharacter_SB* Char = Cast<APlayerCharacter_SB>(GetPawn());
	if (!Char || !UIManager || !Char->GetBuildComponent()) return;

	UIManager->ToggleBuildMenu(Char->GetBuildComponent());
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

void APlayerController_SB::ApplyOverlayInputState()
{
	if (!IsLocalController()) return;

	auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem) return;

	const bool bOverlayOpen = bMenuOpen || bFullMapOpen;

	if (bOverlayOpen)
	{
		if (IMC_Movement) Subsystem->RemoveMappingContext(IMC_Movement);
		if (IMC_Abilities) Subsystem->RemoveMappingContext(IMC_Abilities);

		SetIgnoreMoveInput(true);
		SetIgnoreLookInput(true);

		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(Mode);

		bShowMouseCursor = true;
	}
	else
	{
		if (IMC_Movement) Subsystem->AddMappingContext(IMC_Movement, 0);
		if (IMC_Abilities) Subsystem->AddMappingContext(IMC_Abilities, 0);

		SetIgnoreMoveInput(false);
		SetIgnoreLookInput(false);

		FInputModeGameOnly Mode;
		SetInputMode(Mode);

		bShowMouseCursor = false;
	}
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

void APlayerController_SB::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocalController()) return;
	if (!MapWorldManager || !UIManager) return;

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	const FVector WorldLoc = ControlledPawn->GetActorLocation();
	FVector2D PlayerUV = MapWorldManager->WorldToUV(WorldLoc);

	PlayerUV.X = FMath::Clamp(PlayerUV.X, 0.f, 1.f);
	PlayerUV.Y = FMath::Clamp(PlayerUV.Y, 0.f, 1.f);

	const float PlayerYaw = ControlledPawn->GetActorRotation().Yaw;

	if (UUW_UIHUD* HUD = UIManager->GetHUD())
	{
		if (UUW_Minimap* MinimapWidget = HUD->GetMiniMapWidget())
		{
			MinimapWidget->UpdateMapOffset(PlayerUV);
			MinimapWidget->UpdatePlayerIconRotation(PlayerYaw);

			if (HasPing())
			{
				MinimapWidget->UpdatePing(GetPingUV(), PlayerUV);
			}
			else
			{
				MinimapWidget->ClearPing();
			}
		}
	}
}

void APlayerController_SB::ToggleFullMap()
{
	if (!UIManager) return;

	UIManager->ToggleFullMap();

	UUW_FullMap* FullMap = UIManager->GetFullMapWidget();
	if (!FullMap) return;

	bFullMapOpen = FullMap->IsInViewport();

	if (FullMap->IsInViewport())
	{
		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(FullMap->TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		SetInputMode(Mode);
		bShowMouseCursor = true;

		GetWorldTimerManager().SetTimer(
			FullMapUpdateTimer,
			this,
			&APlayerController_SB::UpdateFullMap,
			0.05f,
			true
		);
	}
	else
	{
		FInputModeGameOnly Mode;
		SetInputMode(Mode);
		bShowMouseCursor = false;

		GetWorldTimerManager().ClearTimer(FullMapUpdateTimer);
	}
}

void APlayerController_SB::SetPing(const FVector2D& InUV)
{
	if (bHasPing)
	{
		if (FVector2D::Distance(CurrentPingUV, InUV) < PingToggleThreshold)
		{
			ClearPing();
			return;
		}
	}

	CurrentPingUV = InUV;
	bHasPing = true;
}

void APlayerController_SB::ClearPing()
{
	bHasPing = false;
}

void APlayerController_SB::UpdateGatherUI()
{
	if (!bGathering || !UIManager)
	{
		GetWorldTimerManager().ClearTimer(GatherUpdateTimer);
		return;
	}

	float Elapsed = GetWorld()->GetTimeSeconds() - GatherStartTime;
	float Percent = Elapsed / GatherDuration;

	Percent = FMath::Clamp(Percent, 0.f, 1.f);

	UIManager->UpdateGatherProgress(Percent);

	float Remaining = GatherDuration - Elapsed;
	Remaining = FMath::Max(Remaining, 0.f);

	UIManager->UpdateGatherTime(Remaining);

	if (Percent >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(GatherUpdateTimer);
	}
}

void APlayerController_SB::UpdateFullMap()
{
	if (!MapWorldManager || !UIManager) return;

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	FVector2D PlayerUV = MapWorldManager->WorldToUV(ControlledPawn->GetActorLocation());

	if (UUW_FullMap* FullMap = UIManager->GetFullMapWidget())
	{
		FullMap->UpdatePlayerPosition(PlayerUV);

		if (HasPing())
		{
			FullMap->UpdatePing(GetPingUV());
		}
		else
		{
			FullMap->ClearPing();
		}
	}
}

void APlayerController_SB::StartGatherProgress(float Duration)
{
	GatherDuration = Duration;
	GatherStartTime = GetWorld()->GetTimeSeconds();
	bGathering = true;

	if (UIManager)
	{
		UIManager->ShowGatherProgress();
	}

	GetWorldTimerManager().SetTimer(
		GatherUpdateTimer,
		this,
		&APlayerController_SB::UpdateGatherUI,
		0.01f,
		true
	);
}

void APlayerController_SB::EndGatherProgress()
{
	bGathering = false;

	GetWorldTimerManager().ClearTimer(GatherUpdateTimer);

	if (UIManager)
	{
		UIManager->HideGatherProgress();
	}
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

bool APlayerController_SB::IsGameplayInputBlocked() const
{
	return UIManager && UIManager->IsMenuBlockingGameplay();
}
