// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GameAbility/MeleeAbility/GA_MeleeLight.h"

#include "GameplayTagContainer.h"

UGA_MeleeLight::UGA_MeleeLight()
{
    // 라이트 공격 프로파일 키
    AttackTag = FGameplayTag::RequestGameplayTag(TEXT("Attack.Light"), false);

    // (선택) 디버그/분류용 Ability 태그
    // AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Melee.Light"), false));

    // (선택) 라이트 공격 중 중복 발동 막고 싶으면(몽타주/스윕 중 재입력 방지)
    // ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Attack"), false));
    // ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("State.Attack"), false));
}