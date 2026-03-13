
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/BaseCharacter_SB.h"
#include "AI/EnemyAIController.h"
#include "AI/EnemyVisualRow.h"
#include "Components/WidgetComponent.h"
#include "Components/SceneComponent.h"
#include "Data/ItemData.h"
#include "EnemyCharacter.generated.h"

class APickup;
class UDataTable;

USTRUCT(BlueprintType)
struct FEnemyDropItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
    int32 MinCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
    int32 MaxCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
    float DropChance = 1.0f;
};

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

    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    UAnimMontage* GetGetHitMontage() const { return GetHitMontage; }

    UFUNCTION(BlueprintCallable, Category = "Enemy|Data")
    void ApplyVisualFromDataTable();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void HandleDeath();

private:
	UPROPERTY(EditDefaultsOnly, Category="Enemy|Data")
	TObjectPtr<UDataTable> EnemyVisualDataTable;

    UPROPERTY(EditAnywhere, Category = "Enemy|Data")
    int32 EnemyId = 0;

    UPROPERTY(VisibleAnywhere, Category = "Enemy|AI")
    EEnemyAttackType AttackType = EEnemyAttackType::Melee;

    UPROPERTY(VisibleAnywhere, Category = "Enemy|AI")
    float PreferredAttackRange = 80.f;

	UPROPERTY(VisibleAnywhere, Category="Enemy|Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

    UPROPERTY(VisibleAnywhere, Category = "Enemy|Combat")
    TObjectPtr<UAnimMontage> DeathMontage;

    UPROPERTY(VisibleAnywhere, Category = "Enemy|Combat")
    TObjectPtr<UAnimMontage> GetHitMontage;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    bool bIsDead = false;

    UPROPERTY()
    TObjectPtr<AEnemyAIController> AIController;

    UPROPERTY()
    TObjectPtr<UBlackboardComponent> BlackboardComp;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Drop", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataTable> ItemDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Drop", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<APickup> PickupClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Drop", meta = (AllowPrivateAccess = "true"))
    TArray<FEnemyDropItem> DropItems;

    void SpawnDropItems();

public:
    UFUNCTION(BlueprintCallable, Category = "Enemy|Data")
    void SetEnemyId(int32 NewId);

    UFUNCTION(BlueprintPure, Category = "Enemy|Data")
    int32 GetEnemyId() const { return EnemyId; }

    UFUNCTION(BlueprintPure, Category = "Enemy|AI")
    bool IsRangedEnemy() const { return AttackType == EEnemyAttackType::Ranged; }

    UFUNCTION(BlueprintPure, Category = "Enemy|AI")
    float GetPreferredAttackRange() const { return PreferredAttackRange; }

    //============ UI=========
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class ADamageNumberActor> DamageNumberClass;

    void SpawnDamageText(float Damage);

    UPROPERTY(VisibleAnywhere, Category = "UI")
    TObjectPtr<UWidgetComponent> AlertWidgetComponent;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> AlertAnchor;

    FTimerHandle AlertHideTimer;

public:
    void ShowAlert();

private:
    void HideAlert();
};
