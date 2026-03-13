
#include "BossAI/BTS_BossUpdateCombatData.h"
#include "BossAI/BossAIController.h"
#include "BossAI/BossAICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/PlayerCharacter_SB.h"
#include "Kismet/GameplayStatics.h"

UBTS_BossUpdateCombatData::UBTS_BossUpdateCombatData()
{
	NodeName = TEXT("Boss Update Combat Data");
	Interval = 0.2f;
	RandomDeviation = 0.0f;
}

void UBTS_BossUpdateCombatData::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	ABossAIController* BossController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	ABossAICharacter* Boss = BossController ? Cast<ABossAICharacter>(BossController->GetPawn()) : nullptr;

	if (!BB || !BossController || !Boss)
	{
		return;
	}

	APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(UGameplayStatics::GetPlayerCharacter(Boss, 0));
	if (!Player)
	{
		BB->ClearValue(ABossAIController::TargetActorKey);
		BB->SetValueAsFloat(ABossAIController::DistanceToTargetKey, 0.f);
		BB->SetValueAsBool(ABossAIController::InMeleeRangeKey, false);
		BB->SetValueAsBool(ABossAIController::InRangedRangeKey, false);
		BB->SetValueAsBool(ABossAIController::ShouldReturnHomeKey, true);
		return;
	}

	const FVector SpawnLocation = BB->GetValueAsVector(ABossAIController::SpawnLocationKey);
	const float DistFromSpawnToPlayer = FVector::Dist(SpawnLocation, Player->GetActorLocation());
	const float DistFromBossToPlayer = FVector::Dist(Boss->GetActorLocation(), Player->GetActorLocation());

	const bool bInsideDetectRange = DistFromSpawnToPlayer <= Boss->GetDetectRange();
	const bool bInsideLoseRange = DistFromSpawnToPlayer <= Boss->GetLoseRange();

	// Ã³À½ °¨Áö
	if (!BB->GetValueAsObject(ABossAIController::TargetActorKey) && bInsideDetectRange)
	{
		BB->SetValueAsObject(ABossAIController::TargetActorKey, Player);
	}

	// ¹üÀ§ ÀÌÅ» ½Ã Å¸°Ù ÇØÁ¦ + º¹±Í
	if (!bInsideLoseRange)
	{
		BB->ClearValue(ABossAIController::TargetActorKey);
		BB->SetValueAsFloat(ABossAIController::DistanceToTargetKey, 0.f);
		BB->SetValueAsBool(ABossAIController::InMeleeRangeKey, false);
		BB->SetValueAsBool(ABossAIController::InRangedRangeKey, false);
		BB->SetValueAsBool(ABossAIController::ShouldReturnHomeKey, true);
		return;
	}

	AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(ABossAIController::TargetActorKey));
	if (!CurrentTarget)
	{
		BB->SetValueAsFloat(ABossAIController::DistanceToTargetKey, 0.f);
		BB->SetValueAsBool(ABossAIController::InMeleeRangeKey, false);
		BB->SetValueAsBool(ABossAIController::InRangedRangeKey, false);
		BB->SetValueAsBool(ABossAIController::ShouldReturnHomeKey, false);
		return;
	}

	BB->SetValueAsFloat(ABossAIController::DistanceToTargetKey, DistFromBossToPlayer);

	const bool bInMelee = DistFromBossToPlayer <= Boss->GetMeleeAttackRange();
	const bool bInRanged = DistFromBossToPlayer > Boss->GetMeleeAttackRange()
		&& DistFromBossToPlayer <= Boss->GetRangedAttackRange();

	BB->SetValueAsBool(ABossAIController::InMeleeRangeKey, bInMelee);
	BB->SetValueAsBool(ABossAIController::InRangedRangeKey, bInRanged);
	BB->SetValueAsBool(ABossAIController::ShouldReturnHomeKey, false);

	UE_LOG(LogTemp, Warning, TEXT("[BossService] Target=%s Dist=%.1f Melee=%d Ranged=%d Return=%d"),
		*GetNameSafe(Cast<AActor>(BB->GetValueAsObject(ABossAIController::TargetActorKey))),
		BB->GetValueAsFloat(ABossAIController::DistanceToTargetKey),
		BB->GetValueAsBool(ABossAIController::InMeleeRangeKey),
		BB->GetValueAsBool(ABossAIController::InRangedRangeKey),
		BB->GetValueAsBool(ABossAIController::ShouldReturnHomeKey));
}