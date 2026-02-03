// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NPCCharacter.h"
#include "NPC/NPCAIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ANPCCharacter::ANPCCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = ANPCAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Pawn,
		ECollisionResponse::ECR_Block);

}

// Called when the game starts or when spawned
void ANPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	ANPCAIController* AIController = GetNPCAIController();
	if (AIController)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] NPC Character initialized"), *NPCName);
	}

	
}

// Called every frame
void ANPCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MonitorStateChanges();

}

bool ANPCCharacter::StartInteraction_Implementation(AActor* Interactor)
{
	//유효성
	if (bIsInteracting || !Interactor)
	{
		return false;
	}

	//거리체크
	float Distance = FVector::Dist(GetActorLocation(), Interactor->GetActorLocation());
	if (Distance > InteractionDistance)
	{
		return false;
	}
	//상호작용
	bIsInteracting = true;
	CurrentInteractor = Interactor;

	ANPCAIController* AIController = GetNPCAIController();
	if (AIController)
	{
		AIController->SetNPCState(ENPCMode::Interacting);
		AIController->SetInteractionTarget(Interactor);
	}

	OnInteractionStarted(Interactor);
	UE_LOG(LogTemp, Log, TEXT("[%s] Interaction started with %s"),
		*NPCName, *Interactor->GetName());

	return true;
}

void ANPCCharacter::EndInteraction_Implementation(AActor* Interactor)
{
	if (!bIsInteracting)
	{
		return;
	}
	bIsInteracting = false;
	AActor* PreviousInteractor = CurrentInteractor;
	CurrentInteractor = nullptr;

	ANPCAIController* AIController = GetNPCAIController();
	if (AIController)
	{
		AIController->SetInteractionTarget(nullptr);
		
		if (AIController->GetTargetActor())
		{
			AIController->SetNPCState(ENPCMode::Alert);
		}
		else
		{
			AIController->SetNPCState(ENPCMode::Idle);
		}

		OnInteractionEnded(PreviousInteractor);
		UE_LOG(LogTemp, Log, TEXT("[%s] Interaction ended"), *NPCName);

	}

}

bool ANPCCharacter::CanInteraction_Implementation(AActor* Interactor) const
{
	if (bIsInteracting || !Interactor)
	{
		return false;
	}

	float Distance = FVector::Dist(GetActorLocation(), Interactor->GetActorLocation());
	if (Distance > InteractionDistance)
	{
		return false;
	}

	ANPCAIController* AIController = GetNPCAIController();
	if (AIController)
	{
		ENPCMode CurrentState = AIController->GetNPCState();

		// Idle이나 Alert 상태에서만 상호작용 가능
		return CurrentState == ENPCMode::Idle || CurrentState == ENPCMode::Alert;
	}
	return false;
}

ANPCAIController* ANPCCharacter::GetNPCAIController() const
{
	return Cast<ANPCAIController>(GetController());
}

float ANPCCharacter::GetInteractionDistance_Implementation() const
{
	return 200.0f;
}

FText ANPCCharacter::GetInteractionText_Implementation(AActor* Interactor) const
{
	return FText::FromString(TEXT("상호작용"));
}

bool ANPCCharacter::IsInteracting_Implementation() const
{
	return false;
}


void ANPCCharacter::MonitorStateChanges()
{
	ANPCAIController* AIController = GetNPCAIController();
	if (!AIController) return;

	uint8 CurrentState = static_cast<uint8>(AIController->GetNPCState());

	if (CurrentState != LastNPCState)
	{
		if(CurrentState == static_cast<uint8>(ENPCMode::Alert) &&
			LastNPCState == static_cast<uint8>(ENPCMode::Idle))
		{
			AActor* Target = AIController->GetTargetActor();
			if (Target)
			{
				OnPlayerDetected(Target);
			}
		}
	}
	else if (CurrentState == static_cast<uint8>(ENPCMode::Alert) &&
		LastNPCState == static_cast<uint8>(ENPCMode::Idle))
	{
		OnPlayerLost();
	}
	LastNPCState = CurrentState;
}
