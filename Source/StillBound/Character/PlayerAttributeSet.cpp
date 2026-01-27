
#include "Character/PlayerAttributeSet.h"
#include "GameplayEffectExtension.h"


UPlayerAttributeSet::UPlayerAttributeSet()
{
}

void UPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetMaxHealthAttribute())
    {
        NewValue = FMath::Max(NewValue, 1.0f);
    }
    else if (Attribute == GetMaxStaminaAttribute())
    {
        NewValue = FMath::Max(NewValue, 0.0f);
    }
    else if (Attribute == GetLevelAttribute())
    {
        NewValue = FMath::Max(NewValue, 1.0f);
        NewValue = FMath::FloorToFloat(NewValue);
    }
    else if (Attribute == GetExperienceAttribute())
    {
        NewValue = FMath::Max(NewValue, 0.0f);
    }

    ClampAttribute(Attribute, NewValue);
}

void UPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    const FGameplayAttribute& AffectedAttr = Data.EvaluatedData.Attribute;

    if (AffectedAttr == GetDamageAttribute())
    {
        const float LocalDamage = GetDamage();
        SetDamage(0.f);

        if (LocalDamage > 0.f)
        {
            const float Reduced = FMath::Max(LocalDamage - GetDefense(), 0.f);

            const float NewHealth = FMath::Clamp(GetHealth() - Reduced, 0.f, GetMaxHealth());
            SetHealth(NewHealth);
        }

        ClampCurrentValues();
        return;
    }

    ClampCurrentValues();
}

void UPlayerAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);

    if (Attribute == GetHealthAttribute())
    {
        OnHealthChanged.Broadcast(OldValue, NewValue);
    }
    else if (Attribute == GetMaxHealthAttribute())
    {
        const float Clamped = FMath::Clamp(GetHealth(), 0.f, GetMaxHealth());
        if (!FMath::IsNearlyEqual(GetHealth(), Clamped))
        {
            SetHealth(Clamped);
        }
        OnMaxHealthChanged.Broadcast(OldValue, NewValue);
    }
    else if (Attribute == GetStaminaAttribute())
    {
        OnStaminaChanged.Broadcast(OldValue, NewValue);
    }
    else if (Attribute == GetMaxStaminaAttribute())
    {
        const float Clamped = FMath::Clamp(GetStamina(), 0.f, GetMaxStamina());
        if (!FMath::IsNearlyEqual(GetStamina(), Clamped))
        {
            SetStamina(Clamped);
        }
        OnMaxStaminaChanged.Broadcast(OldValue, NewValue);
    }
    else if (Attribute == GetLevelAttribute())
    {
        OnLevelChanged.Broadcast(OldValue, NewValue);
    }
    else if (Attribute == GetExperienceAttribute())
    {
        OnExperienceChanged.Broadcast(OldValue, NewValue);
    }
    else if (Attribute == GetAttackAttribute())
    {
        OnAttackChanged.Broadcast(OldValue, NewValue);
    }
    else if (Attribute == GetDefenseAttribute())
    {
        OnDefenseChanged.Broadcast(OldValue, NewValue);
    }
}

void UPlayerAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
    }
    else if (Attribute == GetStaminaAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
    }
    else if (Attribute == GetExperienceAttribute())
    {
        NewValue = FMath::Max(NewValue, 0.f);
    }
    else if (Attribute == GetLevelAttribute())
    {
        NewValue = FMath::Max(FMath::FloorToFloat(NewValue), 1.f);
    }
    else if (Attribute == GetAttackAttribute())
    {
        NewValue = FMath::Max(NewValue, 0.f);
    }
    else if (Attribute == GetDefenseAttribute())
    {
        NewValue = FMath::Max(NewValue, 0.f);
    }
}

void UPlayerAttributeSet::ClampCurrentValues()
{
    SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
    SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));

    SetExperience(FMath::Max(GetExperience(), 0.f));
    SetLevel(FMath::Max(FMath::FloorToFloat(GetLevel()), 1.f));

    SetAttack(FMath::Max(GetAttack(), 0.f));
    SetDefense(FMath::Max(GetDefense(), 0.f));
}