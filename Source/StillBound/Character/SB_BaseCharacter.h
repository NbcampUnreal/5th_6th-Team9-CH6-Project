

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "SB_BaseCharacter.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;

UCLASS()
class STILLBOUND_API ASB_BaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASB_BaseCharacter();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	void GiveStartupAbilities();

private:
	UPROPERTY(EditDefaultsOnly, Category = "SB|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
};
