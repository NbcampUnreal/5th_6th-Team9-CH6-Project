// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "NPCStateEnum.h"
#include "Perception/AIPerceptionTypes.h"
#include "NPCAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UBlackboardComponent;

/*
UENUM(BlueprintType)
enum class ENPCMode:uint8
{
	Idle    UMETA(DisplayName = "대기"),     
	Alert       UMETA(DisplayName = "플레이어 감지"),        // 플레이어를 감지해서 허리 펴고 고개 돌린 상태
	Interacting UMETA(DisplayName = "상호작용 중"),
	Working   UMETA(DisplayName = "작업 중"),    // 다시 작업 자세로 돌아가는 중 (선택)
};
*/

UCLASS()
class STILLBOUND_API ANPCAIController : public AAIController
{
	GENERATED_BODY()

public:
	ANPCAIController();

	//BlackboardKey
	static const FName TargetActorKey;
	static const FName NPCModeKey;
	static const FName InteractionTargetKey;

protected:

	//When Game Started
	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BehaviorTree;

	UPROPERTY(BlueprintReadWrite, Category = "AI")
	UBehaviorTree* BlackboardComponent;

	UPROPERTY(BlueprintReadWrite, Category = "AI")
	UBehaviorTreeComponent* BehaviorTreeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	UAIPerceptionComponent* AIPerception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Sight* SightConfig;

	//When Detect something
	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

public:

	UFUNCTION(BlueprintPure, Category = "AI")
	AActor* GetTargetActor();

	UFUNCTION(BlueprintPure, Category = "AI")
	ENPCMode GetNPCState();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetNPCState(ENPCMode NewState);

	UFUNCTION(BlueprintPure, Category = "AI")
	AActor* GetInteractionTarget();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetInteractionTarget(AActor* Target);
};
