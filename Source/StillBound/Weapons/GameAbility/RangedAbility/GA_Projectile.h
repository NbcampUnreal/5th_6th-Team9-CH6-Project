// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/GameAbility/RangedAbility/WeaponRangedAttackAbilityBase.h"
#include "GA_Projectile.generated.h"

class ARangedWeaponBase;

UCLASS()
class STILLBOUND_API UGA_Projectile : public UWeaponRangedAttackAbilityBase
{
    GENERATED_BODY()

public:
    UGA_Projectile();

protected:
    // 수정: Projectile 전용 모드 검증 + 발사 진입점 override
    virtual void FireCurrentProfile(ARangedWeaponBase* Weapon) override;

    // 수정: Projectile 스폰 / 초기화 / 발사 설정을 이 GA에서 처리
    virtual void FireProjectileOnce(ARangedWeaponBase* Weapon) override;
};