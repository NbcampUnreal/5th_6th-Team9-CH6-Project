
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/BaseCharacter_SB.h"
#include "AI/EnemyAIController.h"
#include "EnemyCharacter.generated.h"


UCLASS()
class STILLBOUND_API AEnemyCharacter : public ABaseCharacter_SB
{
	GENERATED_BODY()
	
public:
    // Sets default values for this character's properties
    AEnemyCharacter();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    virtual void OnConstruction(const FTransform& Transform) override;
public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    UAnimMontage* GetAttackMontage() const { return AttackMontage; }

    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    UAnimMontage* GetDeathMontage() const { return DeathMontage; }

    UFUNCTION(BlueprintCallable, Category = "Enemy|Data")
    void ApplyVisualFromDataTable();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void HandleDeath();

private:
	UPROPERTY(EditDefaultsOnly, Category="Enemy|Data")
	TObjectPtr<UDataTable> EnemyVisualDataTable;

    UPROPERTY(EditAnywhere, Category = "Enemy|Data")
    int32 EnemyId = 0;

	UPROPERTY(VisibleAnywhere, Category="Enemy|Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

    UPROPERTY(VisibleAnywhere, Category = "Enemy|Combat")
    TObjectPtr<UAnimMontage> DeathMontage;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    bool bIsDead = false;

    UPROPERTY()
    TObjectPtr<AEnemyAIController> AIController;

    UPROPERTY()
    TObjectPtr<UBlackboardComponent> BlackboardComp;
};
