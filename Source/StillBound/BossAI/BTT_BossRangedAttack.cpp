
#include "BossAI/BTT_BossRangedAttack.h"
#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

UBTT_BossRangedAttack::UBTT_BossRangedAttack()
{
	NodeName = TEXT("Boss Ranged Attack");
}

EBTNodeResult::Type UBTT_BossRangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	FGameplayTag AbilityTag = FGameplayTag::RequestGameplayTag(TEXT("Boss.Ability.RangedAttack"));
	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));

	return bActivated ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}