

#include "AI/BTTask_EnemyAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "Kismet/KismetMathLibrary.h"

UBTTask_EnemyAttack::UBTTask_EnemyAttack()
{
	NodeName = TEXT("EnemyAttack");
}

EBTNodeResult::Type UBTTask_EnemyAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	APawn* OwnerPawn = AICon->GetPawn();
	if (!OwnerPawn) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (TargetActor)
	{
		FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(
			OwnerPawn->GetActorLocation(),
			TargetActor->GetActorLocation()
		);

		LookAtRot.Pitch = 0.f;
		LookAtRot.Roll = 0.f;

		OwnerPawn->SetActorRotation(LookAtRot);
		AICon->SetControlRotation(LookAtRot);
	}

	if (!AttackAbilityClass) return EBTNodeResult::Failed;

	UAbilitySystemComponent* SourceASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPawn);
	if (!SourceASC) return EBTNodeResult::Failed;

	const bool bActivated = SourceASC->TryActivateAbilityByClass(AttackAbilityClass);

	return bActivated ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}