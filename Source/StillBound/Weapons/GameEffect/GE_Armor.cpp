// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GameEffect/GE_Armor.h"

#include "Character/PlayerAttributeSet.h"

UGE_Armor::UGE_Armor()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UPlayerAttributeSet::GetDefenseAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SBC;
	SBC.DataTag = FGameplayTag::RequestGameplayTag(TEXT("Data.ArmorDefense"), false);

	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SBC);

	Modifiers.Add(Modifier);
}