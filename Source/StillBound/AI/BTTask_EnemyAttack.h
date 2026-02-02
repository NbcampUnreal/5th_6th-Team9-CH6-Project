
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_EnemyAttack.generated.h"

class UGameplayAbility;

UCLASS()
class STILLBOUND_API UBTTask_EnemyAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
    UBTTask_EnemyAttack();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "GAS")
	TSubclassOf<UGameplayAbility> AttackAbilityClass;
};
