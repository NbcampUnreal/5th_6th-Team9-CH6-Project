// Fill out your copyright notice in the Description page of Project Settings.
#include "Weapons/MeleeWeaponBase/MeleeWeaponBase.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"

AMeleeWeaponBase::AMeleeWeaponBase()
{
    // ? CHANGED: PrimaryAttackInputTag / PrimaryAttackAbilityClass 같은 “자동 연결”을 쓰지 않는다.
      // 즉, 여기서 InputTag.Attack.Primary를 기억할 필요가 없다.
}



bool AMeleeWeaponBase::GetAttackProfile(FGameplayTag AttackTag, FWeaponAttackProfile& OutProfile) const
{
    if (!AttackTag.IsValid())
    {
        return false;
    }

    const FWeaponAttackProfile* Found = AttackProfiles.Find(AttackTag);
    if (!Found)
    {
        return false;
    }

    OutProfile = *Found;
    return true;
}
