// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GameEffect/GE_RestoreHealth_Instant.h"
#include "Character/PlayerAttributeSet.h"
#include "GameplayTagContainer.h"

UGE_RestoreHealth_Instant::UGE_RestoreHealth_Instant()
{
    DurationPolicy = EGameplayEffectDurationType::Instant;

    FGameplayModifierInfo Mod;
    Mod.Attribute = UPlayerAttributeSet::GetHealthAttribute();
    Mod.ModifierOp = EGameplayModOp::Additive;

    FSetByCallerFloat SBC;
    SBC.DataTag = FGameplayTag::RequestGameplayTag(TEXT("Data.RestoreHealth"), /*ErrorIfNotFound*/ false);

    // 여기서 SetByCaller 타입으로 세팅됨 (MagnitudeCalculationType을 직접 만질 필요 없음)
    Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);

    Modifiers.Add(Mod);
}