
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"


UCLASS()
class STILLBOUND_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:

    AEnemyAIController();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Behavior")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(BlueprintReadWrite, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(BlueprintReadWrite, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Patrol")
    float PatrolRadius = 1000.f;

    static const FName HomeLocationKey;
    static const FName PatrolLocationKey;
    static const FName TargetActorKey;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
    UAIPerceptionComponent* PerceptionComp;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
    UAISenseConfig_Sight* SightConfig;

protected:
    virtual void BeginPlay() override;

    virtual void OnPossess(APawn* InPawn) override;

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
