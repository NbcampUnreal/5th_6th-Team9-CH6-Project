
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BossAIController.generated.h"

class UBehaviorTree;
class UBehaviorTreeComponent;
class UBlackboardComponent;

UCLASS()
class STILLBOUND_API ABossAIController : public AAIController
{
	GENERATED_BODY()

public:
	ABossAIController();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Behavior")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(BlueprintReadWrite, Category = "AI")
	UBehaviorTreeComponent* BehaviorTreeComponent;

	UPROPERTY(BlueprintReadWrite, Category = "AI")
	UBlackboardComponent* BlackboardComponent;

	static const FName SpawnLocationKey;
	static const FName TargetActorKey;
	static const FName DistanceToTargetKey;
	static const FName InMeleeRangeKey;
	static const FName InRangedRangeKey;
	static const FName ShouldReturnHomeKey;

protected:
	virtual void OnPossess(APawn* InPawn) override;
};
