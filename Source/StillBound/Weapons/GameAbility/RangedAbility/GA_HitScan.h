// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/GameAbility/RangedAbility/WeaponRangedAttackAbilityBase.h"
#include "GA_Hitscan.generated.h"

/**
 * 기본 히트스캔 공격 GA (RangedAttackAbilityBase 설정값만 고정)
 * - FireTag      : Attack.Primary
 * - FireEventTag : Event.Ranged.Fire
 */
UCLASS()
class STILLBOUND_API UGA_Hitscan : public UWeaponRangedAttackAbilityBase
{
    GENERATED_BODY()

public:
    UGA_Hitscan();
};