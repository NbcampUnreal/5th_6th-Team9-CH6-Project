// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GameEffect/GE_RestoreStamina_Instant.h"
#include "Character/PlayerAttributeSet.h"
#include "GameplayTagContainer.h"

UGE_RestoreStamina_Instant::UGE_RestoreStamina_Instant()
{
    DurationPolicy = EGameplayEffectDurationType::Instant;

    FGameplayModifierInfo Mod;
    Mod.Attribute = UPlayerAttributeSet::GetStaminaAttribute();
    Mod.ModifierOp = EGameplayModOp::Additive;

    FSetByCallerFloat SBC;
    SBC.DataTag = FGameplayTag::RequestGameplayTag(TEXT("Data.RestoreStamina"), false);

    Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);
    Modifiers.Add(Mod);
}
