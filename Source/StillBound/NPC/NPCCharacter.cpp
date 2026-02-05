// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NPCCharacter.h"
#include "NPC/NPCAIController.h"
#include "Components/CapsuleComponent.h"
#include "DialogueWidget.h"
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

FInteractableData ANPCCharacter::GetInteractableData_Implementation()
{
	return FInteractableData();
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
		DialogueComponent->OnDialogueStarted.AddDynamic(this, &ANPCCharacter::HandleDialogueStarted);
		DialogueComponent->OnDialogueUpdated.AddDynamic(this, &ANPCCharacter::HandleDialogueUpdated);
		DialogueComponent->OnDialogueEnded.AddDynamic(this, &ANPCCharacter::HandleDialogueEnded);
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

	bool bDialogueStarted = false;
	if (DialogueComponent)
	{
		bDialogueStarted = DialogueComponent->StartDialogue(Interactor);
	}

	OnInteractionStarted(Interactor);
	UE_LOG(LogTemp, Log, TEXT("[%s] Interaction started with %s"),
		*NPCName, *Interactor->GetName());

	if (bDialogueStarted)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Dialogue started successfully"), *NPCName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to start dialogue"), *NPCName);
	}
	return true;
}

void ANPCCharacter::EndInteraction_Implementation(AActor* Interactor)
{
	/*
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
	*/

	// 플레이어가 멀어지거나 고개를 돌렸을 때 실행됨
	UE_LOG(LogTemp, Log, TEXT("플레이어가 떠났습니다. 대화를 강제로 종료합니다."));

	// 여기서 대화창 UI를 끕니다.
	if (DialogueWidget)
	{
		DialogueWidget->CloseDialogue();
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

void ANPCCharacter::BeginFocus_Implementation()
{
	// 플레이어가 NPC를 바라볼 때
	// UI 표시 등
	UE_LOG(LogTemp, Log, TEXT("[%s] Player looking at me"), *NPCName);
}

void ANPCCharacter::EndFocus_Implementation()
{
	// 플레이어가 시선을 돌렸을 때
	UE_LOG(LogTemp, Log, TEXT("[%s] Player looking away"), *NPCName);
}

void ANPCCharacter::BeginInteract_Implementation()
{
	// 상호작용 시작 (버튼 누름)
	UE_LOG(LogTemp, Log, TEXT("[%s] Interaction starting"), *NPCName);
}

void ANPCCharacter::EndInteract_Implementation()
{
	// 상호작용 취소
	UE_LOG(LogTemp, Log, TEXT("[%s] Interaction cancelled"), *NPCName);
}

void ANPCCharacter::Interact_Implementation(AActor* InteractorActor)
{
	APlayerCharacter_SB* PlayerCharacter = Cast<APlayerCharacter_SB>(InteractorActor);
	// 실제 상호작용 실행
	if (!PlayerCharacter) return;

	// 기존 ISB_InteractableInterface 함수 호출
	StartInteraction_Implementation(PlayerCharacter);

	UE_LOG(LogTemp, Log, TEXT("[%s] Interaction executed!"), *NPCName);

	if (DialogueComponent)
	{
		DialogueComponent->StartDialogue(InteractorActor);
	}
}

void ANPCCharacter::HandleDialogueStarted(const FDialogueRow& DialogueData)
{
	if (DialogueWidget == nullptr && DialogueWidgetClass)
	{
		DialogueWidget = CreateWidget<UDialogueWidget>(GetWorld(), DialogueWidgetClass);
		DialogueWidget->OnOptionClicked.AddDynamic(this, &ANPCCharacter::HandleOptionSelected);
	}

	if (DialogueWidget)
	{
		DialogueWidget->AddToViewport();
		// 2. 내용 채우기
		HandleDialogueUpdated(DialogueData);
	}

	//마우스 커서
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(DialogueWidget->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void ANPCCharacter::HandleDialogueUpdated(const FDialogueRow& DialogueData)
{
	if (DialogueWidget)
	{
		// 구조체의 Options를 FText 배열로 변환
		TArray<FText> OptionTexts;
		for (const FDialogueOption& Option : DialogueData.Options)
		{
			OptionTexts.Add(Option.OptionText);
		}

		DialogueWidget->UpdateContent(
			DialogueData.NPCName,
			DialogueData.DialogueText,
			OptionTexts
		);
	}
}


void ANPCCharacter::HandleDialogueEnded()
{
	if (DialogueWidget)
	{
		DialogueWidget->CloseDialogue();
		DialogueWidget = nullptr;
	}
}

void ANPCCharacter::HandleOptionSelected(int32 OptionIndex)
{
	if (DialogueComponent)
	{
		
		DialogueComponent->SelectOption(OptionIndex);
	}
}