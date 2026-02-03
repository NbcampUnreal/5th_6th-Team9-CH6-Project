// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/BTTask_Interact.h"
#include "InteractableInterface.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

UBTTask_Interact::UBTTask_Interact()
{
	NodeName = "Interact";
	bNotifyTick = true;

	InteractionTargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Interact, InteractionTargetKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_Interact::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//경과시간
	ElapsedTime = 0.f;
	bInteractionStarted = false;

	AAIController* AAIController = OwnerComp.GetAIOwner();
	if (!AAIController)
	{
		UE_LOG(LogTemp, Error, TEXT("BTTask_Interact: No AI Controller"));
		return EBTNodeResult::Failed;
	}
	APawn* ControlledPawn = AAIController->GetPawn();
	if (!ControlledPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("BTTask_Interact: No Pawn"));
		return EBTNodeResult::Failed;
	}
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		UE_LOG(LogTemp, Error, TEXT("BTTask_Interact: No Blackboard"));
		return EBTNodeResult::Failed;
	}
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(InteractionTargetKey.SelectedKeyName));
	if (!TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_Interact: No Target actor"));
		return EBTNodeResult::Failed;
	}

	IInteractableInterface* Interactable = Cast<IInteractableInterface>(TargetActor);
	if (!Interactable)
	{
		UE_LOG(LogTemp, Error, TEXT("BTTask_Interact: Target does not implement InteracableInterface"));
		return EBTNodeResult::Failed;
	}

	//거리 체크
	if (bCheckDistance)
	{
		float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
		float MaxDistance = IInteractableInterface::Execute_GetInteractionDistance(TargetActor);

		if (Distance > MaxDistance)
		{
			UE_LOG(LogTemp, Warning, TEXT("BTTask_Interact: Target too far(%.1f>%.1f)"), Distance, MaxDistance);
			return EBTNodeResult::Failed;
		}
	}
	//상호작용 가능 여부 확인
	if (!IInteractableInterface::Execute_CanInteraction(TargetActor, ControlledPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_Interact: Cannot interact with target"));
		return EBTNodeResult::Failed;
	}
	//타겟 보깅
	if (bLookAtTarget)
	{
		AAIController->SetFocus(TargetActor);
	}
	//상호작용 시작
	bInteractionStarted = IInteractableInterface::Execute_StartInteraction(TargetActor, ControlledPawn);

	if (!bInteractionStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_Interact: StartInteraction failed"));
		return EBTNodeResult::Failed;
	}
	UE_LOG(LogTemp, Log, TEXT("BTTask_Interact: Interaction started with %s"), *TargetActor->GetName());

	//지속시간이 0이면 즉시 종료 ->흐름 끊기지 않기 위함..나중에 즉시 행동 구현시(ex. 템줍기, 스위치 켜기 등)
	if (InteractionDuration <= 0.f)
	{
		if (bAutoEndInteraction)
		{
			IInteractableInterface::Execute_EndInteraction(TargetActor, ControlledPawn);
		}
		if (bLookAtTarget)
		{
			AAIController->ClearFocus(EAIFocusPriority::Gameplay);
		}
		return EBTNodeResult::Succeeded;
	}
	//Tick에서 처리..?
	return EBTNodeResult::InProgress;
}

void UBTTask_Interact::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	if (!bInteractionStarted)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;

	}
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(InteractionTargetKey.SelectedKeyName));
	if (!TargetActor)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	IInteractableInterface* Interactable = Cast<IInteractableInterface>(TargetActor);
	if (!Interactable)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	if (bLookAtTarget)
	{
		FVector Direction = TargetActor->GetActorLocation() - ControlledPawn->GetActorLocation();
		Direction.Z = 0.f;
		FRotator TargetRotation = Direction.Rotation();
		FRotator CurrentRotation = ControlledPawn->GetActorRotation();

		FRotator NewRotation = FMath::RInterpTo(
			CurrentRotation,
			FRotator(CurrentRotation.Pitch, TargetRotation.Yaw, CurrentRotation.Roll),
			DeltaSeconds,
			5.f
		);  
		ControlledPawn->SetActorRotation(NewRotation);
	}

	ElapsedTime += DeltaSeconds;
	if (ElapsedTime >= InteractionDuration)
	{
		if (bAutoEndInteraction)
		{
			IInteractableInterface::Execute_EndInteraction(TargetActor, ControlledPawn);
			UE_LOG(LogTemp, Log, TEXT("BTTask_Interact: Interaction ended (auto)"));
		}
	}
	if (bLookAtTarget)
	{
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}
