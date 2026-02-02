

#include "AI/EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

const FName AEnemyAIController::HomeLocationKey = "HomeLocation";
const FName AEnemyAIController::PatrolLocationKey = "PatrolLocation";
const FName AEnemyAIController::TargetActorKey = "TargetActor";

AEnemyAIController::AEnemyAIController()
{
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));

	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	SightConfig->SightRadius = 1500.f;
	SightConfig->LoseSightRadius = 2000.f;
	SightConfig->PeripheralVisionAngleDegrees = 70.f;
	SightConfig->SetMaxAge(5.0f);

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
}

void AEnemyAIController::BeginPlay()
{
    Super::BeginPlay();

	if (PerceptionComp)
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::OnTargetPerceptionUpdated);
	}
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

	if (!InPawn)
	{
		return;
	}

	if (!BehaviorTree || !BehaviorTree->BlackboardAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAIController] BehaviorTree or BlackboardAsset is null."));
		return;
	}

	const bool bBBInit = UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComponent);
	if (!bBBInit || !BlackboardComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAIController] UseBlackboard failed."));
		return;
	}

	//  Home 위치 저장 (배치 위치 기준 정찰)
	const FVector Home = InPawn->GetActorLocation();
	BlackboardComponent->SetValueAsVector(HomeLocationKey, Home);

	BlackboardComponent->SetValueAsVector(PatrolLocationKey, Home);

	// BT 실행
	RunBehaviorTree(BehaviorTree);

	UE_LOG(LogTemp, Warning, TEXT("[EnemyAIController] Possessed. HomeLocation=%s Radius=%.1f"),
		*Home.ToString(), PatrolRadius);
}

void AEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!BlackboardComponent || !Actor)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		BlackboardComponent->SetValueAsObject(TargetActorKey, Actor);
	}
	else
	{
		if (BlackboardComponent->GetValueAsObject(TargetActorKey) == Actor)
		{
			BlackboardComponent->ClearValue(TargetActorKey);
		}
	}
}
