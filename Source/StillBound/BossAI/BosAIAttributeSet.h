
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BosAIAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class STILLBOUND_API UBosAIAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, Category = "SB|Boss|Attribute")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UBosAIAttributeSet, Health);

	UPROPERTY(BlueprintReadOnly, Category = "SB|Boss|Attribute")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UBosAIAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, Category = "SB|Boss|Attribute")
	FGameplayAttributeData Attack;
	ATTRIBUTE_ACCESSORS(UBosAIAttributeSet, Attack);

	UPROPERTY(BlueprintReadOnly, Category = "SB|Boss|Attribute")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UBosAIAttributeSet, Defense);

	UPROPERTY(BlueprintReadOnly, Category = "SB|Boss|Attribute")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UBosAIAttributeSet, Damage);

	UPROPERTY(BlueprintReadOnly, Category = "SB|Boss|Attribute")
	FGameplayAttributeData Phase;
	ATTRIBUTE_ACCESSORS(UBosAIAttributeSet, Phase);

private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};