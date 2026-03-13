
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_BossMeleeAttack.generated.h"

UCLASS()
class STILLBOUND_API UBTT_BossMeleeAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_BossMeleeAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};