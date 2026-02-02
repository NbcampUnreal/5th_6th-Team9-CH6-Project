// Fill out your copyright notice in the Description page of Project Settings.
#include "Weapons/MeleeWeaponBase/MeleeWeaponBase.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"

AMeleeWeaponBase::AMeleeWeaponBase()
{
    // 태그가 아직 ini에 없을 수도 있으니 false(없으면 Invalid로 남음)
    PrimaryAttackInputTag = FGameplayTag::RequestGameplayTag(TEXT("InputTag.Attack.Primary"), false);

    // PrimaryAttackAbilityClass는 파생(Club)에서 세팅하거나 BP에서 세팅
}

void AMeleeWeaponBase::Equip(AActor* NewOwner, UAbilitySystemComponent* InASC)
{
    // 장착 직전에 “입력태그 -> GA” 매핑이 누락되어 있으면 자동으로 추가/교체
    EnsurePrimaryAttackGrant();

    Super::Equip(NewOwner, InASC);
}

bool AMeleeWeaponBase::RequestPrimaryAttack()
{
    return PrimaryAttackInputTag.IsValid() && ActivateByInputTag(PrimaryAttackInputTag);
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

void AMeleeWeaponBase::EnsurePrimaryAttackGrant()
{
    if (!PrimaryAttackInputTag.IsValid() || !PrimaryAttackAbilityClass)
    {
        return;
    }

    // GrantedAbilities는 AWeaponBase에 이미 존재(Equip 시 ASC에 GiveAbility됨).
    // 여기서 “PrimaryAttackInputTag 항목”이 없으면 추가하고,
    // 있으면 AbilityClass를 최신 값으로 교체한다.
    for (FWeaponAbilityGrant& G : GrantedAbilities)
    {
        if (G.InputTag == PrimaryAttackInputTag)
        {
            G.Ability = PrimaryAttackAbilityClass;
            G.AbilityLevel = FMath::Max(1, G.AbilityLevel);
            return;
        }
    }

    FWeaponAbilityGrant NewGrant;
    NewGrant.InputTag = PrimaryAttackInputTag;
    NewGrant.Ability = PrimaryAttackAbilityClass;
    NewGrant.AbilityLevel = 1;
    GrantedAbilities.Add(NewGrant);
}