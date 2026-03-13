
#include "BossAI/BTT_BossMeleeAttack.h"
#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

UBTT_BossMeleeAttack::UBTT_BossMeleeAttack()
{
	NodeName = TEXT("Boss Melee Attack");
}

EBTNodeResult::Type UBTT_BossMeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AICon->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	FGameplayTag AbilityTag = FGameplayTag::RequestGameplayTag(TEXT("Boss.Ability.MeleeAttack"));
	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));

	return bActivated ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}