

#include "Character/PlayerCharacter_SB.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Character/PlayerAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Inventory/InventoryComponent.h"
#include "Interface/InteractionInterface.h"
#include "NPC/DialogueComponent.h"
#include "DrawDebugHelpers.h"
#include "UI/USB_UIManager.h"
#include "Components/SlateWrapperTypes.h"
#include "Character/PlayerController_SB.h"
#include "Items/Pickup.h"

#include "Weapons/WeaponBase.h"


APlayerCharacter_SB::APlayerCharacter_SB()
{
	PrimaryActorTick.bCanEverTick = true;

	//  플레이어 AttributeSet 생성
	PlayerAttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("PlayerAttributeSet"));

	//  InitStats는 이 클래스로
	AttributeSetClassForInitStats = UPlayerAttributeSet::StaticClass();

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 97.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>("FollowCamera");
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//minimap camera
	MiniMapArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("MiniMapArm"));
	MiniMapArm->SetupAttachment(GetRootComponent());
	MiniMapArm->SetRelativeRotation(FRotator(-90.f, 0, 0));
	MiniMapArm->bDoCollisionTest = false;

	MiniMapCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MiniMapCapture"));
	MiniMapCapture->SetupAttachment(MiniMapArm);
	MiniMapCapture->ProjectionType = ECameraProjectionMode::Orthographic;

	PlayerInventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("PlayerInventory"));
	PlayerInventory->SetSlotsCapacity(20);
	PlayerInventory->SetWeightCapacity(50.f);


	InteractionCheckFrequency = 0.1f;
	InteractionCheckDistance = 225.f;

}

FInteractableData APlayerCharacter_SB::GetInteractableData_Implementation()
{
	return FInteractableData();
}

void APlayerCharacter_SB::BeginPlay()
{
	Super::BeginPlay();

	if (!AbilitySystemComponent) return;

	const UPlayerAttributeSet* AS = AbilitySystemComponent->GetSet<UPlayerAttributeSet>();

	UE_LOG(LogTemp, Warning, TEXT("[Player] ASC Set<UPlayerAttributeSet>=%p"), AS);

	const float H = AbilitySystemComponent->GetNumericAttribute(UPlayerAttributeSet::GetHealthAttribute());
	const float MH = AbilitySystemComponent->GetNumericAttribute(UPlayerAttributeSet::GetMaxHealthAttribute());

	UE_LOG(LogTemp, Warning, TEXT("[Player] After InitStats H=%.1f / %.1f"), H, MH);


	if (MiniMapTarget)
	{
		MiniMapCapture->TextureTarget = MiniMapTarget;
	}

	// ? 시작 무기 장착
	EquipStartingWeapon();

}

void APlayerCharacter_SB::EquipStartingWeapon()
{
	if (EquippedWeapon) return;
	if (!StartingWeaponClass) return;
	if (!AbilitySystemComponent) return;

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp) return;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(StartingWeaponClass, Params);
	if (!NewWeapon) return;

	// 1) 손 소켓에 부착
	NewWeapon->AttachToComponent(
		MeshComp,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		StartingWeaponSocketName
	);

	// (선택) 무기 충돌 끄고 싶으면
	// NewWeapon->SetActorEnableCollision(false);

	// 2) ASC에 무기 GA/GE 부여 (Spec.SourceObject=this(weapon) 포함)
	NewWeapon->Equip(this, AbilitySystemComponent);

	EquippedWeapon = NewWeapon;

	UE_LOG(LogTemp, Log, TEXT("[Player] StartingWeapon Equipped: %s -> Socket(%s)"),
		*GetNameSafe(NewWeapon), *StartingWeaponSocketName.ToString());

	UE_LOG(LogTemp, Warning, TEXT("[Equip] ASC=%s"), *GetNameSafe(AbilitySystemComponent));


}


void APlayerCharacter_SB::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
	{
		PerformInteractionCheck();
	}
	if (InteractionData.bIsInteracting && InteractionData.CurrentInteractable)
	{
		float Dist = FVector::Dist(GetActorLocation(), InteractionData.CurrentInteractable->GetActorLocation());

		if (Dist > 300.0f)
		{
			EndInteract(); 
		}
	}
}

void APlayerCharacter_SB::PerformInteractionCheck()
{
	if (InteractionData.bIsInteracting) return;

	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	FVector TraceStart{ GetPawnViewLocation() };
	FVector TraceEnd{ TraceStart + (GetViewRotation().Vector() * InteractionCheckDistance) };

	float LookDirection = FVector::DotProduct(GetActorForwardVector(), GetViewRotation().Vector());

	if (LookDirection > 0)
	{
		DrawDebugLine(
			GetWorld(),
			TraceStart,
			TraceEnd,
			FColor::Red,
			false,
			1.f,
			2.f);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		FHitResult TraceHit;

		if (GetWorld()->LineTraceSingleByChannel(
			TraceHit,
			TraceStart,
			TraceEnd,
			ECC_Visibility,
			QueryParams))
		{
			AActor* HitActor = TraceHit.GetActor();

			// [핵심] 1. 나 자신(this)이면 무시, 2. 유효한 액터인지 확인
			if (HitActor && HitActor != this)
			{
				// 인터페이스를 가지고 있는?
				if (HitActor->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
				{
					// 새로운 액터-> FoundInteractable 호출
					if (HitActor != InteractionData.CurrentInteractable)
					{
						FoundInteractable(HitActor);
					}

					// 어떤 경우간에 인터페이스가 있는 액터를 봤으니
					// 밑에 있는 NoInteractableFound()가 실행 안 되도록 탈출
					return;
				}
			}
		}
	}

	NoInteractableFound();
}

void APlayerCharacter_SB::FoundInteractable(AActor* NewInteractable)
{
	if (IsInteracting())
	{
		EndInteract();
	}

	if (InteractionData.CurrentInteractable)
	{
		TargetInteractable = InteractionData.CurrentInteractable;
	
		IInteractionInterface::Execute_EndFocus(TargetInteractable.GetObject());
	}

	InteractionData.CurrentInteractable = NewInteractable;
	TargetInteractable = NewInteractable;

	auto* PC = Cast<APlayerController_SB>(GetController());
	if (!PC) return;

	USB_UIManager* UI = PC->UIManager;
	if(!UI) return;


	if (TargetInteractable.GetObject())
	{
		FInteractableData Data = IInteractionInterface::Execute_GetInteractableData(TargetInteractable.GetObject());
		UI->UpdateInteractionWidget(Data);
		IInteractionInterface::Execute_BeginFocus(TargetInteractable.GetObject());

	}

}

void APlayerCharacter_SB::NoInteractableFound()
{
	if (InteractionData.bIsInteracting && !InteractionData.CurrentInteractable)
	{
		EndInteract();
		return;
	}

	if (InteractionData.CurrentInteractable)
	{
		if (IsValid(TargetInteractable.GetObject()))
		{
			IInteractionInterface::Execute_EndFocus(TargetInteractable.GetObject());
		}

		auto* PC = Cast<APlayerController_SB>(GetController());
		if (!PC) return;

		USB_UIManager* UI = PC->UIManager;
		if (!UI) return;

		UI->HideInteractionWidget();

		InteractionData.CurrentInteractable = nullptr;
		TargetInteractable = nullptr;
	}
}

void APlayerCharacter_SB::BeginInteract()
{

	InteractionData.bIsInteracting = true;
	//verify nothing has changed with the interactble state since beginning interaction
	PerformInteractionCheck();

	if (!InteractionData.CurrentInteractable || !IsValid(TargetInteractable.GetObject()))
	{
		if (auto* PC = Cast<APlayerController_SB>(GetController()))
		{
			if (PC->UIManager)
			{
				PC->UIManager->HideInteractionWidget();
			}
		}
		return;
	}

	


	if (InteractionData.CurrentInteractable)
	{
		if (IsValid(TargetInteractable.GetObject()))
		{
			IInteractionInterface::Execute_BeginInteract(TargetInteractable.GetObject());
			FInteractableData TargetData = IInteractionInterface::Execute_GetInteractableData(TargetInteractable.GetObject());
			if (FMath::IsNearlyZero(TargetData.InteractionDuration, 0.1f))
			{
				Interact();
			}
			else
			{
				GetWorldTimerManager().SetTimer(
					TimerHandle_Interaction,
					this,
					&APlayerCharacter_SB::Interact,
					TargetData.InteractionDuration, // 받아온 데이터의 시간 사용
					false);
			}
		}
	}
}

void APlayerCharacter_SB::EndInteract()
{
	if (InteractionData.CurrentInteractable)
	{
		IInteractionInterface::Execute_EndInteract(InteractionData.CurrentInteractable);
	}

	InteractionData.bIsInteracting = false;
	GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);

	InteractionData.CurrentInteractable = nullptr;
	TargetInteractable = nullptr;

	if (auto* PC = Cast<APlayerController_SB>(GetController()))
	{
		if (PC->UIManager)
		{
			PC->UIManager->HideInteractionWidget();
		}
	}
}

void APlayerCharacter_SB::Interact()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);

	UObject* TargetObject = TargetInteractable.GetObject();
	if (!TargetObject)
	{
		UE_LOG(LogTemp, Error, TEXT("Interact: TargetInteractable.GetObject() is nullptr"));
		return;
	}

	AActor* TargetActor = Cast<AActor>(TargetObject);
	if (!IsValid(TargetActor))
	{
		UE_LOG(LogTemp, Error, TEXT("Interact: TargetActor is invalid"));
		return;
	}

	if (!TargetActor->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("Interact: Target does not implement IInteractionInterface"));
		return;
	}

	// 1. NPC인지 확인
	UDialogueComponent* DialogueComp = TargetActor->FindComponentByClass<UDialogueComponent>();
	if (DialogueComp)
	{
		// 상호작용 프롬프트 숨김
		if (auto* PC = Cast<APlayerController_SB>(GetController()))
		{
			if (PC->UIManager)
			{
				PC->UIManager->HideInteractionWidget();
			}
		}

		// 종료 바인딩
		DialogueComp->OnDialogueEnded.RemoveAll(this);
		DialogueComp->OnDialogueEnded.AddDynamic(this, &APlayerCharacter_SB::EndInteract);
	
		InteractionData.bIsInteracting = true;
	}
		IInteractionInterface::Execute_Interact(TargetActor, this);
		//아이템일때
		if (DialogueComp == nullptr)
		{
			EndInteract();
		}
}

void APlayerCharacter_SB::UpdateInteractionWidget() const
{
	UObject* InteractableObject = TargetInteractable.GetObject();

	if (IsValid(TargetInteractable.GetObject()))
	{
		auto* PC = Cast<APlayerController_SB>(GetController());
		if (!PC) return;

		USB_UIManager* UI = PC->UIManager;
		if (!UI) return;

		FInteractableData Data = IInteractionInterface::Execute_GetInteractableData(InteractableObject);
		UI->UpdateInteractionWidget(Data);
	}
}

void APlayerCharacter_SB::DropItem(UItemBase* ItemToDrop, const int32 QuantityToDrop)
{
	if (PlayerInventory->FindMatchingItem(ItemToDrop))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		const FVector SpawnLocation{ GetActorLocation() + (GetActorForwardVector() * 50.f) };
		const FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

		const int32 RemovedQuantity = PlayerInventory->RemoveAmountOfItem(ItemToDrop, QuantityToDrop);

		APickup* Pickup = GetWorld()->SpawnActor<APickup>(APickup::StaticClass(), SpawnTransform, SpawnParams);

		Pickup->InitializeDrop(ItemToDrop, RemovedQuantity);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Item to drop was shomhow null."));
	}
}

void APlayerCharacter_SB::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->DisableMovement();
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
		{
			Anim->StopAllMontages(0.1f);
		}
	}

	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage, 1.5f);
		return;
	}
}
