

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "BaseCharacter_SB.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;
class UDataTable;
class UPlayerAttributeSet;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SB|GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SB|GAS")
	TObjectPtr<UDataTable> DefaultAttributeMetaDataTable;

	UPROPERTY(VisibleAnywhere, Category = "SB|Attribute")
	TObjectPtr<class UPlayerAttributeSet> PlayerAttributeSet;

private:
	UPROPERTY(EditDefaultsOnly, Category = "SB|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	bool bAbilitiesGiven = false;
};
