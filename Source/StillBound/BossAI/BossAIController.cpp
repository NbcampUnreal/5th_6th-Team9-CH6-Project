
#include "BossAI/BossAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

const FName ABossAIController::SpawnLocationKey = "SpawnLocation";
const FName ABossAIController::TargetActorKey = "TargetActor";
const FName ABossAIController::DistanceToTargetKey = "DistanceToTarget";
const FName ABossAIController::InMeleeRangeKey = "bInMeleeRange";
const FName ABossAIController::InRangedRangeKey = "bInRangedRange";
const FName ABossAIController::ShouldReturnHomeKey = "bShouldReturnHome";

ABossAIController::ABossAIController()
{
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
}

void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn)
	{
		return;
	}

	if (!BehaviorTree || !BehaviorTree->BlackboardAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossAIController] BehaviorTree or BlackboardAsset is null."));
		return;
	}

	const bool bBBInit = UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComponent);
	if (!bBBInit || !BlackboardComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossAIController] UseBlackboard failed."));
		return;
	}

	const FVector SpawnLocation = InPawn->GetActorLocation();

	BlackboardComponent->SetValueAsVector(SpawnLocationKey, SpawnLocation);
	BlackboardComponent->ClearValue(TargetActorKey);
	BlackboardComponent->SetValueAsFloat(DistanceToTargetKey, 0.f);
	BlackboardComponent->SetValueAsBool(InMeleeRangeKey, false);
	BlackboardComponent->SetValueAsBool(InRangedRangeKey, false);
	BlackboardComponent->SetValueAsBool(ShouldReturnHomeKey, false);

	RunBehaviorTree(BehaviorTree);

	UE_LOG(LogTemp, Warning, TEXT("[BossAIController] RunBehaviorTree: %s"), *GetNameSafe(BehaviorTree));
}