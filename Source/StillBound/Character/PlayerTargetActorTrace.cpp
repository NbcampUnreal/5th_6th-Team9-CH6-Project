

#include "Character/PlayerTargetActorTrace.h"
#include "Abilities/GameplayAbility.h"

APlayerTargetActorTrace::APlayerTargetActorTrace()
{
}

void APlayerTargetActorTrace::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	SourceActor = Ability -> GetCurrentActorInfo() -> AvatarActor.Get();

}

void APlayerTargetActorTrace::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData());
	if (IsConfirmTargetingAllowed())
	{
		FHitResult HitResult = GetSphereTraceResult(SourceActor);
		FGameplayAbilityTargetDataHandle DataHandle(new FGameplayAbilityTargetData_SingleTargetHit(HitResult));
		TargetDataReadyDelegate.Broadcast(DataHandle);
	}
}