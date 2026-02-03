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
	//상호작용할 액터
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector InteractionTargetKey;

	//상호작용 지속 시간 
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "0.0"))
	float InteractionDuration = 3.0f;

	/**
 * 상호작용 전 거리 체크 수행 여부
 */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	bool bCheckDistance = true;

	/**
 * 상호작용 중 타겟을 바라볼지 여부
 */
	UPROPERTY(EditAnywhere, Category = "Interaction")
	bool bLookAtTarget = true;

	//상호작용 종료 시 자동으로 EndInteraction 호출 여부
	UPROPERTY(EditAnywhere, Category = "Interaction")
	bool bAutoEndInteraction = true;

private:
	//경과시간
	float ElapsedTime = 0.f;

	//상호작용 시작 성공 여부
	bool bInteractionStarted = false;
	
};
