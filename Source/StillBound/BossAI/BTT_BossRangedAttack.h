
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "BTT_BossRangedAttack.generated.h"

UCLASS()
class STILLBOUND_API UBTT_BossRangedAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_BossRangedAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Boss|Ability")
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere, Category = "Boss|Ability")
	FGameplayTag CooldownTag;
};
