// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/GameAbility/RangedAbility/WeaponRangedAttackAbilityBase.h"
#include "GA_HitScan.generated.h"

/**
 * Hitscan(Trace) 발사 GA
 * - 프로파일 선택: AbilitySpec.DynamicAbilityTags의 InputTag.* 로 결정됨
 * - 여기서는 FireEventTag/디버그 등 "프리셋"만 설정
 */
UCLASS()
class STILLBOUND_API UGA_HitScan : public UWeaponRangedAttackAbilityBase
{
    GENERATED_BODY()

public:
    UGA_HitScan();
};