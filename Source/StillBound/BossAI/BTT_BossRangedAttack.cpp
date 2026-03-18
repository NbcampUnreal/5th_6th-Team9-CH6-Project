
#include "BossAI/BTT_BossRangedAttack.h"
#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

UBTT_BossRangedAttack::UBTT_BossRangedAttack()
{
	NodeName = TEXT("Boss Ranged Attack");

	AbilityTag = FGameplayTag::RequestGameplayTag(TEXT("Boss.Ability.RangedAttack"));
	CooldownTag = FGameplayTag::RequestGameplayTag(TEXT("Boss.State.ShootCooldown"));
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

	if (CooldownTag.IsValid() && ASC->HasMatchingGameplayTag(CooldownTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossRangedTask] Cooldown active -> Failed"));
		return EBTNodeResult::Failed;
	}

	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));

	UE_LOG(LogTemp, Warning, TEXT("[BossRangedTask] TryActivate %s"), *AbilityTag.ToString());
	UE_LOG(LogTemp, Warning, TEXT("[BossRangedTask] Activated=%d"), bActivated);

	return bActivated ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}