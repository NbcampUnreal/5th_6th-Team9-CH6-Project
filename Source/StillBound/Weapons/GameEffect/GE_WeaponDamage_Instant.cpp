// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GameEffect/GE_WeaponDamage_Instant.h"

#include "Character/PlayerAttributeSet.h"
#include "GameplayEffectTypes.h"        // FSetByCallerFloat
#include "GameplayTagContainer.h"

UGE_WeaponDamage_Instant::UGE_WeaponDamage_Instant()
{
    // 즉발(Instant)
    DurationPolicy = EGameplayEffectDurationType::Instant;

    // Modifier: Target의 Damage(메타) += SetByCaller(Data.Damage)
    FGameplayModifierInfo Mod;
    Mod.Attribute = UPlayerAttributeSet::GetDamageAttribute();
    Mod.ModifierOp = EGameplayModOp::Additive;

    // SetByCaller(Data.Damage) 설정
    FSetByCallerFloat SBC;
    SBC.DataTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Damage"), /*ErrorIfNotFound*/ false);

    ensureMsgf(SBC.DataTag.IsValid(),
        TEXT("[GAS] GameplayTag 'Data.Damage' is not registered. Add it in Project Settings > GameplayTags"));

    // SetByCaller는 ModifierMagnitude에 FSetByCallerFloat를 넣으면 된다.
    Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);

    Modifiers.Add(Mod);
}