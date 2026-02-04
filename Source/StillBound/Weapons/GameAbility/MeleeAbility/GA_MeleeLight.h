// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/GameAbility/MeleeAbility/WeaponMeleeAttackAbilityBase.h"
#include "GA_MeleeLight.generated.h"

/**
 * 기본 근접 라이트 공격 GA
 * - AttackTag = Attack.Light 프로파일을 무기에서 가져와서 실행
 */
UCLASS()
class STILLBOUND_API UGA_MeleeLight : public UWeaponMeleeAttackAbilityBase
{
    GENERATED_BODY()

public:
    UGA_MeleeLight();
};