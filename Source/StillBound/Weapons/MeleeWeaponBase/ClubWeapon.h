// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/MeleeWeaponBase/MeleeWeaponBase.h"
#include "ClubWeapon.generated.h"

UCLASS(Blueprintable)
class STILLBOUND_API AClubWeapon : public AMeleeWeaponBase
{
    GENERATED_BODY()

public:
    AClubWeapon();

protected:
    /** 기본으로 제공할 공격 태그(프로파일 키). 예: Attack.Light */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee|Club")
    FGameplayTag LightAttackTag;
};
