
#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter_SB.h"
#include "BossAICharacter.generated.h"

class UBosAIAttributeSet;
class UAnimMontage;

UCLASS()
class STILLBOUND_API ABossAICharacter : public ABaseCharacter_SB
{
	GENERATED_BODY()

public:
	ABossAICharacter();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "Boss|AI")
	float GetDetectRange() const { return DetectRange; }

	UFUNCTION(BlueprintPure, Category = "Boss|AI")
	float GetLoseRange() const { return LoseRange; }

	UFUNCTION(BlueprintPure, Category = "Boss|AI")
	float GetMeleeAttackRange() const { return MeleeAttackRange; }

	UFUNCTION(BlueprintPure, Category = "Boss|AI")
	float GetRangedAttackRange() const { return RangedAttackRange; }

	UFUNCTION(BlueprintCallable, Category = "Boss|Death")
	void HandleDeath();

	UFUNCTION(BlueprintPure, Category = "Boss|Death")
	UAnimMontage* GetDeathMontage() const { return DeathMontage; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Attributes")
	TObjectPtr<UBosAIAttributeSet> BossAttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Death")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|AI")
	float DetectRange = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|AI")
	float LoseRange = 1800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|AI")
	float MeleeAttackRange = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|AI")
	float RangedAttackRange = 1000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Death")
	bool bIsDead = false;
};
