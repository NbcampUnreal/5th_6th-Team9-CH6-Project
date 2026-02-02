
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PlayerAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAttributeDataChanged, float, OldValue, float, NewValue);

UCLASS()
class STILLBOUND_API UPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UPlayerAttributeSet();

    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
    virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

    UPROPERTY(BlueprintAssignable, Category = "SB|Attribute")
    mutable FAttributeDataChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "SB|Attribute")
    mutable FAttributeDataChanged OnMaxHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "SB|Attribute")
    mutable FAttributeDataChanged OnStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category = "SB|Attribute")
    mutable FAttributeDataChanged OnMaxStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category = "SB|Attribute")
    mutable FAttributeDataChanged OnLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "SB|Attribute")
    mutable FAttributeDataChanged OnExperienceChanged;

    UPROPERTY(BlueprintAssignable, Category = "SB|Attribute")
    mutable FAttributeDataChanged OnAttackChanged;

    UPROPERTY(BlueprintAssignable, Category = "SB|Attribute")
    mutable FAttributeDataChanged OnDefenseChanged;

private:
    void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
    void ClampCurrentValues();

public:
    UPROPERTY(BlueprintReadOnly, Category = "SB|Attribute")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Health);

    UPROPERTY(BlueprintReadOnly, Category = "SB|Attribute")
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, MaxHealth);

    UPROPERTY(BlueprintReadOnly, Category = "SB|Attribute")
    FGameplayAttributeData Stamina;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Stamina);

    UPROPERTY(BlueprintReadOnly, Category = "SB|Attribute")
    FGameplayAttributeData MaxStamina;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, MaxStamina);

    UPROPERTY(BlueprintReadOnly, Category = "SB|Attribute")
    FGameplayAttributeData Level;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Level);

    UPROPERTY(BlueprintReadOnly, Category = "SB|Attribute")
    FGameplayAttributeData Experience;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Experience);

    UPROPERTY(BlueprintReadOnly, Category = "SB|Attribute")
    FGameplayAttributeData Attack;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Attack);

    UPROPERTY(BlueprintReadOnly, Category = "SB|Attribute")
    FGameplayAttributeData Defense;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Defense);

    UPROPERTY(BlueprintReadOnly, Category = "SB|Attribute")
    FGameplayAttributeData Damage;
    ATTRIBUTE_ACCESSORS(UPlayerAttributeSet, Damage);

public:

    //레벨별 필요 경험치 계산
    float GetRequiredExpForLevel(int32 InLevel) const;
};