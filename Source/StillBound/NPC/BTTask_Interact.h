// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_Interact.generated.h"

/**
 * Behavior Tree 상호작용 실행
 * InteractableInterface를 구현한 액터와 상호작용
 */
UCLASS(Blueprintable)
class STILLBOUND_API UBTTask_Interact : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_Interact();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    // === Blackboard Key ===
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector InteractionTargetKey;

    // === Interaction Settings ===

    // 상호작용 지속 시간 (0 = 즉시 종료, 대화 등은 수동 종료)
    UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "0.0"))
    float InteractionDuration = 0.0f;

    // 거리 체크 여부
    UPROPERTY(EditAnywhere, Category = "Interaction")
    bool bCheckDistance = true;

    // 타겟을 바라볼지 여부
    UPROPERTY(EditAnywhere, Category = "Interaction")
    bool bLookAtTarget = true;

    // 자동으로 EndInteract 호출 여부 (대화는 false, 아이템 줍기는 true)
    UPROPERTY(EditAnywhere, Category = "Interaction")
    bool bAutoEndInteraction = false;

private:
    // Task 실행 중 유지되는 데이터
    float ElapsedTime = 0.0f;
    bool bInteractionStarted = false;

    // 캐싱된 참조 (TickTask에서 사용)
    UPROPERTY()
    AActor* CachedTargetActor = nullptr;

    UPROPERTY()
    AAIController* CachedAIController = nullptr;

    UPROPERTY()
    APawn* CachedControlledPawn = nullptr;

    // 헬퍼 함수
    bool ValidateInteraction(UBehaviorTreeComponent& OwnerComp);
    void CleanupInteraction();
	
};
