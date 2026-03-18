
#include "BossAI/BTS_BossUpdateCombatData.h"
#include "BossAI/BossAIController.h"
#include "BossAI/BossAICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/PlayerCharacter_SB.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

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

		// 새로 추가한 블랙보드 키
		BB->SetValueAsBool(TEXT("bCanMeleeAttack"), false);
		BB->SetValueAsBool(TEXT("bCanRangedAttack"), false);

		return;
	}

	const FVector SpawnLocation = BB->GetValueAsVector(ABossAIController::SpawnLocationKey);
	const float DistFromSpawnToPlayer = FVector::Dist(SpawnLocation, Player->GetActorLocation());
	const float DistFromBossToPlayer = FVector::Dist(Boss->GetActorLocation(), Player->GetActorLocation());

	const bool bInsideDetectRange = DistFromSpawnToPlayer <= Boss->GetDetectRange();
	const bool bInsideLoseRange = DistFromSpawnToPlayer <= Boss->GetLoseRange();

	// 처음 감지
	if (!BB->GetValueAsObject(ABossAIController::TargetActorKey) && bInsideDetectRange)
	{
		BB->SetValueAsObject(ABossAIController::TargetActorKey, Player);
	}

	// 범위 이탈 시 타겟 해제 + 복귀
	if (!bInsideLoseRange)
	{
		BB->ClearValue(ABossAIController::TargetActorKey);
		BB->SetValueAsFloat(ABossAIController::DistanceToTargetKey, 0.f);
		BB->SetValueAsBool(ABossAIController::InMeleeRangeKey, false);
		BB->SetValueAsBool(ABossAIController::InRangedRangeKey, false);
		BB->SetValueAsBool(ABossAIController::ShouldReturnHomeKey, true);

		BB->SetValueAsBool(TEXT("bCanMeleeAttack"), false);
		BB->SetValueAsBool(TEXT("bCanRangedAttack"), false);

		return;
	}

	AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(ABossAIController::TargetActorKey));
	if (!CurrentTarget)
	{
		BB->SetValueAsFloat(ABossAIController::DistanceToTargetKey, 0.f);
		BB->SetValueAsBool(ABossAIController::InMeleeRangeKey, false);
		BB->SetValueAsBool(ABossAIController::InRangedRangeKey, false);
		BB->SetValueAsBool(ABossAIController::ShouldReturnHomeKey, false);

		BB->SetValueAsBool(TEXT("bCanMeleeAttack"), false);
		BB->SetValueAsBool(TEXT("bCanRangedAttack"), false);

		return;
	}

	BB->SetValueAsFloat(ABossAIController::DistanceToTargetKey, DistFromBossToPlayer);

	// 기존 거리 판정
	const bool bInMelee = DistFromBossToPlayer <= Boss->GetMeleeAttackRange();
	const bool bInRanged = DistFromBossToPlayer > Boss->GetMeleeAttackRange()
		&& DistFromBossToPlayer <= Boss->GetRangedAttackRange();

	BB->SetValueAsBool(ABossAIController::InMeleeRangeKey, bInMelee);
	BB->SetValueAsBool(ABossAIController::InRangedRangeKey, bInRanged);
	BB->SetValueAsBool(ABossAIController::ShouldReturnHomeKey, false);

	// 쿨다운 태그 검사
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Boss);

	const FGameplayTag MeleeCooldownTag =
		FGameplayTag::RequestGameplayTag(TEXT("Boss.State.MeleeCooldown"));

	const FGameplayTag RangedCooldownTag =
		FGameplayTag::RequestGameplayTag(TEXT("Boss.State.ShootCooldown"));

	const bool bMeleeCooldown = ASC ? ASC->HasMatchingGameplayTag(MeleeCooldownTag) : false;
	const bool bRangedCooldown = ASC ? ASC->HasMatchingGameplayTag(RangedCooldownTag) : false;

	// 실제 공격 가능 여부
	const bool bCanMeleeAttack = bInMelee && !bMeleeCooldown;
	const bool bCanRangedAttack = bInRanged && !bRangedCooldown;

	BB->SetValueAsBool(TEXT("bCanMeleeAttack"), bCanMeleeAttack);
	BB->SetValueAsBool(TEXT("bCanRangedAttack"), bCanRangedAttack);

}