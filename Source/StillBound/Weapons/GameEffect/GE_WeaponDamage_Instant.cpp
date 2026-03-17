// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GameEffect/GE_WeaponDamage_Instant.h"

#include "AI/AIAttributeSet.h" 
#include "BossAI/BosAIAttributeSet.h"

#include "GameplayEffectTypes.h"        // FSetByCallerFloat
#include "GameplayTagContainer.h"

UGE_WeaponDamage_Instant::UGE_WeaponDamage_Instant()
{
    // 즉발(Instant)
    DurationPolicy = EGameplayEffectDurationType::Instant;

    // SetByCaller(Data.EnemyDamage)
    FSetByCallerFloat SBC;
    SBC.DataTag = FGameplayTag::RequestGameplayTag(TEXT("Data.EnemyDamage"), false);

    ensureMsgf(SBC.DataTag.IsValid(),
        TEXT("[GAS] GameplayTag 'Data.EnemyDamage' is not registered. Add it in Project Settings > GameplayTags"));

    // =========================
    // 일반 몬스터용 Damage
    // =========================
    {
        FGameplayModifierInfo Mod;
        Mod.Attribute = UAIAttributeSet::GetDamageAttribute();
        Mod.ModifierOp = EGameplayModOp::Additive;
        Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);
        Modifiers.Add(Mod);
    }

    // =========================
    // 보스용 Damage
    // =========================
    {
        FGameplayModifierInfo Mod; // //수정
        Mod.Attribute = UBosAIAttributeSet::GetDamageAttribute(); // //수정
        Mod.ModifierOp = EGameplayModOp::Additive;
        Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);
        Modifiers.Add(Mod);
    }
}