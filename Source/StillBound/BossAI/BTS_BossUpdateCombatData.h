
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_BossUpdateCombatData.generated.h"

UCLASS()
class STILLBOUND_API UBTS_BossUpdateCombatData : public UBTService
{
	GENERATED_BODY()

public:
	UBTS_BossUpdateCombatData();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};