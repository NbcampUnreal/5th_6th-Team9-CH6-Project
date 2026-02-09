// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NPCAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"

const FName ANPCAIController::TargetActorKey(TEXT("TargetActor"));
const FName ANPCAIController::NPCModeKey(TEXT("NPCMode"));
const FName ANPCAIController::InteractionTargetKey(TEXT("InteractionTarget"));

ANPCAIController::ANPCAIController()
{
	//1. Add Component
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightComponent"));

	//2. Set the Sight
	SightConfig->SightRadius = 1000.f;
	SightConfig->LoseSightRadius = 1400.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	//3. Add Settings on the Component
	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ANPCAIController::BeginPlay()
{
    Super::BeginPlay();

    if (AIPerception)
    {
        AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ANPCAIController::OnTargetDetected);
    }
}

void ANPCAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsEnum(NPCModeKey, static_cast<uint8>(ENPCMode::Idle));
        BB->SetValueAsObject(TargetActorKey, nullptr);
        BB->SetValueAsObject(InteractionTargetKey, nullptr);
    }
    // Behavior Tree 실행
    if (BehaviorTree)
    {
        // Blackboard 연결 (있을 때만)
        if (BehaviorTree->BlackboardAsset)
        {
            UseBlackboard(BehaviorTree->BlackboardAsset, BB);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] BT has no Blackboard Asset!"),
                *InPawn->GetName());
        }

        // BT 실행
        RunBehaviorTree(BehaviorTree);

        UE_LOG(LogTemp, Log, TEXT("[%s] AI Controller possessed and BT started"),
            *InPawn->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] No Behavior Tree assigned!"),
            *InPawn->GetName());
    }
}


void ANPCAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }

    bool bIsPlayer = Actor->ActorHasTag(TEXT("Player"));
    bool bIsPawn = Actor->IsA(APawn::StaticClass());

    UBlackboardComponent* BB = GetBlackboardComponent();

    if (!BB)
    {
        return;
    }
        if (Stimulus.WasSuccessfullySensed())
        {
            // 발견
            BB->SetValueAsEnum(NPCModeKey, static_cast<uint8>(ENPCMode::Alert));
            BB->SetValueAsObject(TargetActorKey, Actor);
            SetFocus(Actor);
        }
        else
        {
            // 시야에서 사라짐
            AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TargetActorKey));

            if (CurrentTarget == Actor)
            {
                BB->SetValueAsEnum(NPCModeKey, static_cast<uint8>(ENPCMode::Idle));
                BB->SetValueAsObject(TargetActorKey, nullptr);
                ClearFocus(EAIFocusPriority::Gameplay);

                UE_LOG(LogTemp, Log, TEXT("[%s] Player lost sight"), *GetPawn()->GetName());
            }
        }
    }

AActor* ANPCAIController::GetTargetActor()
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        return Cast<AActor>(BB->GetValueAsObject(TargetActorKey));
    }
    return nullptr;
}

ENPCMode ANPCAIController::GetNPCState()
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        return static_cast<ENPCMode>(BB->GetValueAsEnum(NPCModeKey));
    }
    return ENPCMode::Idle;
}

void ANPCAIController::SetNPCState(ENPCMode NewState)
{
   UBlackboardComponent* BB = GetBlackboardComponent();
   if (BB)
   {
       BB->SetValueAsEnum(NPCModeKey, static_cast<uint8>(NewState));

       UE_LOG(LogTemp, Log, TEXT("[%s] state changed to: %d"), *GetPawn()->GetName(), (int32)NewState)
   }
}

AActor* ANPCAIController::GetInteractionTarget()
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        return Cast<AActor>(BB->GetValueAsObject(InteractionTargetKey));
    }
    return nullptr;
}

void ANPCAIController::SetInteractionTarget(AActor* Target)
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsObject(InteractionTargetKey, Target);

        if (Target)
        {
            UE_LOG(LogTemp, Log, TEXT("[%s] Interaction target set: %s"),
                *GetPawn()->GetName(), *Target->GetName());
        }
    }
}
