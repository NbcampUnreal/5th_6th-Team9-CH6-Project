// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/GameAbility/MeleeAbility/WeaponMeleeAttackAbilityBase.h"
#include "GA_MeleeLight.generated.h"

/**
 * 기본 근접 라이트 공격 GA
 * - 더 이상 AttackTag를 들고 있지 않음(중복 제거)
 * - 어떤 프로파일을 쓸지는 "이 AbilitySpec에 심긴 InputTag.*"로 결정됨
 *   (WeaponBase에서 GiveAbility 할 때 Spec.DynamicAbilityTags에 InputTag를 넣어둠)
 */
UCLASS()
class STILLBOUND_API UGA_MeleeLight : public UWeaponMeleeAttackAbilityBase
{
    GENERATED_BODY()

public:
    UGA_MeleeLight();
};