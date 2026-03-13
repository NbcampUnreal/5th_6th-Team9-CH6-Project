// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/GameAbility/RangedAbility/GA_HitScan.h"
#include "GameplayTagContainer.h"

UGA_HitScan::UGA_HitScan()
{
    // FireTag 제거됨.
    // 어떤 FireProfile을 사용할지는 "현재 AbilitySpec에 심긴 InputTag.*"로 결정:
    //   RangedWeaponBase::FireProfiles[InputTag]  (WeaponRangedAttackAbilityBase 내부에서 조회)

    // 몽타주 노티파이 타이밍 발사 이벤트
    FireEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Ranged.Fire"), false);

    // 몽타주/이벤트가 없을 때 즉시 1회 발사할지(원하면 false)
    bFireImmediatelyIfNoMontageOrEvent = true;

    // 디버그
    bDebugTrace = true;
    DebugLifeTime = 1.0f;
    DebugLineThickness = 1.5f;

    // (선택) 분류용 AbilityTags
    // AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Ranged.Hitscan"), false));
}