
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AIAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class STILLBOUND_API UAIAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, Category = "SB|AI|Attribute")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAIAttributeSet, Health);

	UPROPERTY(BlueprintReadOnly, Category = "SB|AI|Attribute")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAIAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, Category = "SB|AI|Attribute")
	FGameplayAttributeData Attack;
	ATTRIBUTE_ACCESSORS(UAIAttributeSet, Attack);

	UPROPERTY(BlueprintReadOnly, Category = "SB|AI|Attribute")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UAIAttributeSet, Defense);


	UPROPERTY(BlueprintReadOnly, Category = "SB|AI|Attribute")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UAIAttributeSet, Damage);

	UPROPERTY(BlueprintReadOnly, Category = "SB|AI|Attribute")
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS(UAIAttributeSet, Level);

	UPROPERTY(BlueprintReadOnly, Category = "SB|AI|Attribute")
	FGameplayAttributeData ExpReward;
	ATTRIBUTE_ACCESSORS(UAIAttributeSet, ExpReward);

private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
