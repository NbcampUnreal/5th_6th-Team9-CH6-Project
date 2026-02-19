// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/GameAbility/RangedAbility/GA_Hitscan.h"
#include "GameplayTagContainer.h"

UGA_Hitscan::UGA_Hitscan()
{
    // FireProfile 키 (RangedWeaponBase의 FireProfiles에서 이 키로 Profile을 찾음)
    FireTag = FGameplayTag::RequestGameplayTag(TEXT("Attack.Primary"), /*ErrorIfNotFound*/ false);

    // 몽타주 발사 타이밍 이벤트 (AnimNotify에서 SendGameplayEventToActor로 쏘는 태그)
    FireEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Ranged.Fire"), /*ErrorIfNotFound*/ false);

    // (선택) 디버그/분류용 AbilityTags
    // AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Ranged.Hitscan"), false));

    // (선택) 몽타주/이벤트 없으면 즉시 1회 발사
    // bFireImmediatelyIfNoMontageOrEvent = true;

    // (선택) 디버그 라인
    // bDebugTrace = true;
}