

#include "AI/BTTask_RandomPatrolPoint.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "AI/EnemyAIController.h"


UBTTask_RandomPatrolPoint::UBTTask_RandomPatrolPoint()
{
	NodeName = TEXT("Pick Random Patrol Point (Around Home)");
}

EBTNodeResult::Type UBTTask_RandomPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!AICon || !BB) return EBTNodeResult::Failed;

    const FVector Home = BB->GetValueAsVector(HomeLocationKey.SelectedKeyName);

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(AICon->GetWorld());
    if (!NavSys) return EBTNodeResult::Failed;

    FNavLocation OutLoc;

    float Radius = PatrolRadius;
    if (bUseControllerPatrolRadius)
    {
        if (AEnemyAIController* MyCon = Cast<AEnemyAIController>(AICon))
        {
            Radius = MyCon->PatrolRadius;
        }
    }
    const bool bFound = NavSys->GetRandomReachablePointInRadius(Home, Radius, OutLoc);
    if (!bFound) return EBTNodeResult::Failed;

    BB->SetValueAsVector(PatrolLocationKey.SelectedKeyName, OutLoc.Location);
    return EBTNodeResult::Succeeded;
}