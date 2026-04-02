
#include "BossAI/BTT_BossMeleeAttack.h"
#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BossAI/BossAIController.h"
#include "Kismet/KismetMathLibrary.h"

UBTT_BossMeleeAttack::UBTT_BossMeleeAttack()
{
	NodeName = TEXT("Boss Melee Attack");

	AbilityTag = FGameplayTag::RequestGameplayTag(TEXT("Boss.Ability.MeleeAttack"));
	CooldownTag = FGameplayTag::RequestGameplayTag(TEXT("Boss.State.MeleeCooldown"));
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

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = BB
		? Cast<AActor>(BB->GetValueAsObject(ABossAIController::TargetActorKey))
		: nullptr;

	if (TargetActor)
	{
		FVector Start = Pawn->GetActorLocation();
		FVector End = TargetActor->GetActorLocation();

		FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(Start, End);
		LookAtRot.Pitch = 0.f;
		LookAtRot.Roll = 0.f;

		Pawn->SetActorRotation(LookAtRot);
		AICon->SetControlRotation(LookAtRot);
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	// 쿨타임 중이면 아예 근접 공격 시도 안 함
	if (CooldownTag.IsValid() && ASC->HasMatchingGameplayTag(CooldownTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BossMeleeTask] Cooldown active -> Failed"));
		return EBTNodeResult::Failed;
	}

	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));

	UE_LOG(LogTemp, Warning, TEXT("[BossMeleeTask] TryActivate %s"), *AbilityTag.ToString());
	UE_LOG(LogTemp, Warning, TEXT("[BossMeleeTask] Activated=%d"), bActivated);

	return bActivated ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}