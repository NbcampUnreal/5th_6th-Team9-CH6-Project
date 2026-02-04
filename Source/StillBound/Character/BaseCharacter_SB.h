#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AttributeSet.h"
#include "BaseCharacter_SB.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;
class UDataTable;
class UPlayerAttributeSet;
class UInventoryComponent;
class UGameplayEffect;

UCLASS()
class STILLBOUND_API ABaseCharacter_SB : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter_SB();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPlayerAttributeSet* GetPlayerAttributeSet() const;

protected:
	virtual void BeginPlay() override;

	void GiveStartupAbilities();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SB|GAS")
	TSubclassOf<UAttributeSet> AttributeSetClassForInitStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SB|GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SB|GAS")
	TObjectPtr<UDataTable> DefaultAttributeMetaDataTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SB|GAS")
	TObjectPtr<UPlayerAttributeSet> PlayerAttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "SB|GAS")
	TSubclassOf<UGameplayEffect> DefaultStaminaRegenEffect;

private:
	UPROPERTY(EditDefaultsOnly, Category = "SB|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	bool bAbilitiesGiven = false;
};
