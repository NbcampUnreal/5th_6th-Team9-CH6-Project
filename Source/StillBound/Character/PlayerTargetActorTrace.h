
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "PlayerTargetActorTrace.generated.h"



UCLASS()
class STILLBOUND_API APlayerTargetActorTrace : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
	
public:
	APlayerTargetActorTrace();
	
	virtual void StartTargeting(UGameplayAbility* Ability) override;

	virtual void ConfirmTargetingAndContinue() override;

protected:
	UFUNCTION(BlueprintImplementableEvent)
	FHitResult GetSphereTraceResult(AActor* InSourceActor);
};
