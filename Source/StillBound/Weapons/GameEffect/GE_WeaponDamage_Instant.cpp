// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GameEffect/GE_WeaponDamage_Instant.h"

#include "AI/AIAttributeSet.h" 
#include "BossAI/BosAIAttributeSet.h"

#include "GameplayEffectTypes.h"        // FSetByCallerFloat
#include "GameplayTagContainer.h"

UGE_WeaponDamage_Instant::UGE_WeaponDamage_Instant()
{
    // ï¿½ï¿½ï¿?Instant)
    DurationPolicy = EGameplayEffectDurationType::Instant;

    // SetByCaller(Data.EnemyDamage)
    FSetByCallerFloat SBC;
    SBC.DataTag = FGameplayTag::RequestGameplayTag(TEXT("Data.EnemyDamage"), /* ErrorIfNotFound */ false);

    ensureMsgf(SBC.DataTag.IsValid(),
        TEXT("[GAS] GameplayTag 'Data.EnemyDamage' is not registered. Add it in Project Settings > GameplayTags"));


    // ÀÏ¹Ý ¸ó½ºÅÍ¿ë Damage

    // =========================
    {
        FGameplayModifierInfo Mod;
        Mod.Attribute = UAIAttributeSet::GetDamageAttribute();
        Mod.ModifierOp = EGameplayModOp::Additive;
        Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);
        Modifiers.Add(Mod);
    }


    {
        FGameplayModifierInfo Mod; // //¼öÁ¤
        Mod.Attribute = UBosAIAttributeSet::GetDamageAttribute(); // //¼öÁ¤
        Mod.ModifierOp = EGameplayModOp::Additive;
        Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);
        Modifiers.Add(Mod);
    }
}