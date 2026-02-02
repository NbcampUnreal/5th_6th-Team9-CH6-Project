

#include "Character/PlayerCharacter_SB.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Character/PlayerAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Interface/InteractionInterface.h"
#include "DrawDebugHelpers.h"

APlayerCharacter_SB::APlayerCharacter_SB()
{
	PrimaryActorTick.bCanEverTick = true;

	//  플레이어 AttributeSet 생성
	CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("PlayerAttributeSet"));

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

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryCompontnt"));
	
	InteractionCheckFrequency = 0.1f;
	InteractionCheckDistance = 225.f;

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
}

void APlayerCharacter_SB::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
	{
		PerformInteractionCheck();
	}
}

void APlayerCharacter_SB::PerformInteractionCheck()
{
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
			if (TraceHit.GetActor()->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
			{
				const float Distance = (TraceStart - TraceHit.ImpactPoint).Size();

				if (TraceHit.GetActor() != InteractionData.CurrentInteractable && Distance <= InteractionCheckDistance)
				{
					FoundInteractable(TraceHit.GetActor());
					return;
				}

				if (TraceHit.GetActor() == InteractionData.CurrentInteractable)
				{
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
		TargetInteractable->EndFocus();
	}

	InteractionData.CurrentInteractable = NewInteractable;
	TargetInteractable = NewInteractable;

	TargetInteractable->BeginFocus();
}

void APlayerCharacter_SB::NoInteractableFound()
{
	if (IsInteracting())
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);
	}

	if (InteractionData.CurrentInteractable)
	{
		if (IsValid(TargetInteractable.GetObject()))
		{
			TargetInteractable->EndFocus();
		}

		// hide interaction widget on the HUD

		InteractionData.CurrentInteractable = nullptr;
		TargetInteractable = nullptr;
	}
}

void APlayerCharacter_SB::BeginInteract()
{
	//verify nothing has changed with the interactble state since beginning interaction
	PerformInteractionCheck();

	if (InteractionData.CurrentInteractable)
	{
		if (IsValid(TargetInteractable.GetObject()))
		{
			TargetInteractable->BeginInteract();

			if (FMath::IsNearlyZero(TargetInteractable->InteractableData.InteractionDuration, 0.1f))
			{
				Interact();
			}
			else
			{
				GetWorldTimerManager().SetTimer(
					TimerHandle_Interaction,
					this,
					&ThisClass::Interact,
					TargetInteractable->InteractableData.InteractionDuration,
					false);
			}
		}
	}
}

void APlayerCharacter_SB::EndInteract()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);

	if (IsValid(TargetInteractable.GetObject()))
	{
		TargetInteractable->EndInteract();
	}
}

void APlayerCharacter_SB::Interact()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);

	if (IsValid(TargetInteractable.GetObject()))
	{
		TargetInteractable->Interact();
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
