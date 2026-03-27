#include "Weapons/GameEffect/GE_StaminaCost_Instant.h"

#include "Character/PlayerAttributeSet.h"

#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"

UGE_StaminaCost_Instant::UGE_StaminaCost_Instant()
{
    DurationPolicy = EGameplayEffectDurationType::Instant;

    FSetByCallerFloat SBC;
    SBC.DataTag = FGameplayTag::RequestGameplayTag(TEXT("Data.StaminaCost"), false);

    ensureMsgf(
        SBC.DataTag.IsValid(),
        TEXT("[GAS] GameplayTag 'Data.StaminaCost' is not registered. Add it in Project Settings > GameplayTags or DefaultGameplayTags.ini")
    );

    {
        FGameplayModifierInfo Mod;
        Mod.Attribute = UPlayerAttributeSet::GetStaminaAttribute();
        Mod.ModifierOp = EGameplayModOp::Additive;
        Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);
        Modifiers.Add(Mod);
    }
}