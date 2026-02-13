// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NPCCharacter.h"
#include "NPC/NPCAIController.h"
#include "NPC/DialogueComponent.h"
#include "NPC/DialogueWidget.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ANPCCharacter::ANPCCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DialogueComponent = CreateDefaultSubobject<UDialogueComponent>(TEXT("DialogueComponent"));

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

	if (DialogueComponent)
	{
		if (DialogueComponent->MainMenuDataTable)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] MainMenu Table Name: %s"),
				*NPCName, *DialogueComponent->MainMenuDataTable->GetName());
		}
		DialogueComponent->OnDialogueStarted.AddDynamic(this, &ANPCCharacter::OnDialogueStart);
		DialogueComponent->OnDialogueUpdated.AddDynamic(this, &ANPCCharacter::OnDialogueUpdate);
		DialogueComponent->OnDialogueEnded.AddDynamic(this, &ANPCCharacter::OnDialogueEnd);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] DialogueComponent is NULL in BeginPlay!"), *NPCName);
	}
	if (DialogueWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] DialogueWidgetClass: %s"),
			*NPCName, *DialogueWidgetClass->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] DialogueWidgetClass is NULL!"), *NPCName);
	}

	
}

void ANPCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	MonitorStateChanges();

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
	else if (CurrentState == static_cast<uint8>(ENPCMode::Idle) &&
		LastNPCState == static_cast<uint8>(ENPCMode::Alert))
	{
		OnPlayerLost();
	}
	LastNPCState = CurrentState;
}

void ANPCCharacter::BeginFocus_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[%s] Player looking at me"), *NPCName);
}

void ANPCCharacter::EndFocus_Implementation()
{
	// 플레이어가 시선을 돌렸을 때
	UE_LOG(LogTemp, Log, TEXT("[%s] Player looking away"), *NPCName);
}

void ANPCCharacter::EndInteract_Implementation()
{
	if (DialogueComponent && DialogueComponent->IsDialogueActive())
	{
		DialogueComponent->EndDialogue();
	}
}

void ANPCCharacter::Interact_Implementation(APlayerCharacter_SB* PlayerCharacter)
{
	if (!PlayerCharacter || bIsInteracting) return;

	UE_LOG(LogTemp, Log, TEXT("[%s] Interaction Started with %s"),
		*NPCName, *PlayerCharacter->GetName());

	bIsInteracting = true;
	CurrentInteractor = PlayerCharacter;

	// AI 상태 변경
	if (ANPCAIController* AIController = GetNPCAIController())
	{
		AIController->SetNPCState(ENPCMode::Interacting);
		AIController->SetInteractionTarget(PlayerCharacter);
	}

	// 대화 시작 (DialogueComponent가 이벤트 발동)
	if (DialogueComponent)
	{
		bool bStarted = DialogueComponent->StartDialogue(PlayerCharacter);

		if (bStarted)
		{
			OnInteractionStarted(PlayerCharacter);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to start dialogue"), *NPCName);

			// 즉시 정리
			bIsInteracting = false;
			CurrentInteractor = nullptr;

			if (ANPCAIController* AIController = GetNPCAIController())
			{
				AIController->SetInteractionTarget(nullptr);
				AIController->SetNPCState(ENPCMode::Idle);
			}
		}
	}
}

FInteractableData ANPCCharacter::GetInteractableData_Implementation()
{
	FInteractableData Data;

	Data.InteractableType = EInteractableType::NonPlayerCharacter;
	Data.Name = FText::FromString(NPCName); 
	Data.Action = FText::FromString(TEXT("대화하기")); 
	Data.InteractionDuration = 0.0f; 

	return Data;
}

float ANPCCharacter::GetInteractionDistance_Implementation()
{
	return 200.0f;
}

ANPCAIController* ANPCCharacter::GetNPCAIController() const
{
	return Cast<ANPCAIController>(GetController());
}

void ANPCCharacter::OnDialogueStart(const FDialogueRow& DialogueData)
{
	if (!DialogueComponent) return;

	if (!DialogueWidget)
	{
		if (!DialogueWidgetClass) return;

		UE_LOG(LogTemp, Log, TEXT("[%s] Creating new DialogueWidget"), *NPCName);
		DialogueWidget = CreateWidget<UDialogueWidget>(GetWorld(), DialogueWidgetClass);

		if (!DialogueWidget) return;

		// 옵션 클릭 이벤트 바인딩
		DialogueWidget->OnOptionClicked.AddDynamic(this, &ANPCCharacter::OnOptionSelected);
	}
	DialogueWidget->SetDialogueComponent(DialogueComponent);

	// Widget 표시 및 업데이트
	if (!DialogueWidget->IsInViewport())
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Adding DialogueWidget to viewport"), *NPCName);
		DialogueWidget->AddToViewport(100);
	}
	DialogueWidget->ShowDialogue(DialogueData);

}

void ANPCCharacter::OnDialogueUpdate(const FDialogueRow& DialogueData)
{
	if (DialogueWidget && DialogueWidget->IsInViewport())
	{
		DialogueWidget->ShowDialogue(DialogueData);
	}
}

void ANPCCharacter::OnDialogueEnd()
{
	// Widget 정리
	if (DialogueWidget && DialogueWidget->IsInViewport())
	{
		DialogueWidget->RemoveFromParent();
		// Widget은 재사용을 위해 유지 (nullptr 안 함)
	}

	// 상태 정리
	bIsInteracting = false;
	AActor* PreviousInteractor = CurrentInteractor;
	CurrentInteractor = nullptr;

	if (PreviousInteractor)
	{
		if (APlayerController* PC = Cast<APlayerController>(PreviousInteractor->GetInstigatorController()))
		{
			PC->SetShowMouseCursor(false);

			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);

			UE_LOG(LogTemp, Log, TEXT("[%s] Player input mode restored"), *NPCName);
		}
	}

	// AI 상태 복구
	if (ANPCAIController* AIController = GetNPCAIController())
	{
		AIController->SetInteractionTarget(nullptr);

		// 플레이어가 여전히 근처에 있으면 Alert
		if (AIController->GetTargetActor())
		{
			AIController->SetNPCState(ENPCMode::Alert);
		}
		else
		{
			AIController->SetNPCState(ENPCMode::Idle);
		}
	}

	// 블루프린트 이벤트
	OnInteractionEnded(PreviousInteractor);

	UE_LOG(LogTemp, Log, TEXT("[%s] Dialogue ended"), *NPCName);
}

void ANPCCharacter::OnOptionSelected(int32 OptionIndex)
{
	if (DialogueComponent)
	{
		DialogueComponent->SelectOption(OptionIndex);
	}
}