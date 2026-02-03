
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_RandomPatrolPoint.generated.h"


UCLASS()
class STILLBOUND_API UBTTask_RandomPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_RandomPatrolPoint();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HomeLocationKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PatrolLocationKey;

	UPROPERTY(EditAnywhere, Category = "Patrol")
	float PatrolRadius = 100.f;

	UPROPERTY(EditAnywhere, Category = "Patrol")
	bool bUseControllerPatrolRadius = false;
};
