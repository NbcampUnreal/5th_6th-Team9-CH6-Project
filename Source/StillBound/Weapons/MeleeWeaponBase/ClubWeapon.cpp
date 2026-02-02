// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/MeleeWeaponBase/ClubWeapon.h"

#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"

AClubWeapon::AClubWeapon()
{
    // (원하면 네이밍 규칙에 맞게 바꾸세요)
    WeaponTypeTag = FGameplayTag::RequestGameplayTag(TEXT("Weapon.Melee.Club"), false);

    // 장착 소켓 기본값(프로젝트 소켓명에 맞게 수정)
    AttachSocketName = TEXT("WeaponSocket");

    // 이 무기의 “라이트 공격” 프로파일 키
    LightAttackTag = FGameplayTag::RequestGameplayTag(TEXT("Attack.Light"), false);

    // GA는 다음 단계에서 만들 예정이므로 기본은 nullptr
    // - BP_ClubWeapon에서 PrimaryAttackAbilityClass를 UGA_MeleeLight로 지정하거나
    // - UGA_MeleeLight 구현 후 여기서 StaticClass()로 지정하세요.
    PrimaryAttackAbilityClass = nullptr;

    // ===== 기본 라이트 공격 프로파일(샘플) =====
    // 실제 몽타주/이펙트는 블루프린트에서 세팅하는 걸 추천
    if (LightAttackTag.IsValid())
    {
        FWeaponAttackProfile LightProfile;
        LightProfile.Montage = nullptr;         // BP에서 설정
        LightProfile.MontagePlayRate = 1.0f;

        // 몽둥이 느낌: 짧은 리치 + 두꺼운 판정
        LightProfile.Sweep.TraceStartSocket = TEXT("TraceStart");
        LightProfile.Sweep.TraceEndSocket = TEXT("TraceEnd");
        LightProfile.Sweep.MaxDistance = 170.f;
        LightProfile.Sweep.Radius = 18.f;
        LightProfile.Sweep.TraceChannel = ECC_Pawn;
        LightProfile.Sweep.bHitEachActorOnce = true;
        LightProfile.Sweep.bHitFirstTargetOnly = true;

        // 온힛 효과(데미지 GE 등)는 다음 단계/또는 BP에서 구성
        // FOnHitGameplayEffectSpec DamageSpec;
        // DamageSpec.Effect = UGE_WeaponDamage::StaticClass(); // 예시
        // DamageSpec.Level = 1.f;
        // DamageSpec.SetByCallerMagnitudes.Add(DamageTag, 20.f);
        // LightProfile.OnHitTargetEffects.Add(DamageSpec);

        AttackProfiles.Add(LightAttackTag, LightProfile);
    }
}