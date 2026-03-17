// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GameAbility/RangedAbility/GA_Projectile.h"

#include "Weapons/RangedWeapon/RangedWeaponBase.h"
#include "GameplayTagContainer.h"

UGA_Projectile::UGA_Projectile()
{
    // 몽타주 노티파이 타이밍 발사 이벤트
    FireEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Ranged.Fire"), false);

    // 몽타주/이벤트가 없으면 즉시 발사
    bFireImmediatelyIfNoMontageOrEvent = true;

    // 디버그
    bDebugTrace = true;
    DebugLifeTime = 1.0f;
    DebugLineThickness = 1.5f;

    // 선택: 분류용 태그
    // AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Ranged.Projectile"), false));
}

void UGA_Projectile::FireCurrentProfile(ARangedWeaponBase* Weapon)
{
    if (!Weapon || !bHasCachedProfile)
    {
        return;
    }

    // 안전장치:
    // 이 GA를 쓸 때 FireProfile이 Hitscan으로 세팅돼 있으면 projectile 발사를 막는다.
    if (!CachedProfile.IsProjectileMode())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GA_Projectile] FireProfile is not Projectile mode. Weapon=%s Ability=%s"),
            *GetNameSafe(Weapon),
            *GetNameSafe(this));
        return;
    }

    FireProjectileOnce(Weapon);
}