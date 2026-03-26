

#include "AI/EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"
#include "AI/EnemyCharacter.h"
#include "Character/PlayerCharacter_SB.h"
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
	SightConfig->PeripheralVisionAngleDegrees = 180.f;
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

	const FVector Home = InPawn->GetActorLocation();
	BlackboardComponent->SetValueAsVector(HomeLocationKey, Home);

	BlackboardComponent->SetValueAsVector(PatrolLocationKey, Home);

	RunBehaviorTree(BehaviorTree);

	GetWorldTimerManager().ClearTimer(ChaseRangeTimerHandle);
	GetWorldTimerManager().SetTimer(
		ChaseRangeTimerHandle,
		this,
		&ThisClass::CheckChaseRange,
		ChaseRangeCheckInterval,
		true
	);
}

void AEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!BlackboardComponent || !Actor)
	{
		return;
	}

	if (Actor == GetPawn())
	{
		return;
	}

	if (!Stimulus.WasSuccessfullySensed())
	{
		if (BlackboardComponent->GetValueAsObject(TargetActorKey) == Actor)
		{
			BlackboardComponent->ClearValue(TargetActorKey);
		}
		return;
	}

	APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(Actor);
	if (!Player)
	{
		return;
	}

	BlackboardComponent->SetValueAsObject(TargetActorKey, Player);

	AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetPawn());
	if (Enemy)
	{
		Enemy->ShowAlert();
	}
}

void AEnemyAIController::CheckChaseRange()
{
	if (!BlackboardComponent)
	{
		return;
	}

	APawn* MyPawn = GetPawn();
	if (!MyPawn)
	{
		return;
	}

	AActor* Target = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKey));
	if (!Target)
	{
		return;
	}

	const FVector Home = BlackboardComponent->GetValueAsVector(HomeLocationKey);
	const float DistFromHome = FVector::Dist(MyPawn->GetActorLocation(), Home);

	if (DistFromHome > MaxChaseRangeFromHome)
	{
		BlackboardComponent->ClearValue(TargetActorKey);
		BlackboardComponent->SetValueAsVector(PatrolLocationKey, Home);
		StopMovement();
	}
}
