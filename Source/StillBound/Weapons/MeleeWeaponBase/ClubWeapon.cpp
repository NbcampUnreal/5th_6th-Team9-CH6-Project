// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/MeleeWeaponBase/ClubWeapon.h"

#include "GameplayTagContainer.h"

AClubWeapon::AClubWeapon()
{
    // 필요하면 네 태그 규칙에 맞춰 변경
    WeaponTypeTag = FGameplayTag::RequestGameplayTag(TEXT("Weapon.Melee.Club"), false);

    // 이 무기의 라이트 공격 프로파일 키
    LightAttackTag = FGameplayTag::RequestGameplayTag(TEXT("Attack.Light"), false);

    if (LightAttackTag.IsValid())
    {
        FWeaponAttackProfile LightProfile;
        LightProfile.Montage = nullptr;     // BP에서 지정 추천
        LightProfile.MontagePlayRate = 1.0f;

        // ? 오버랩(히트박스) 설정
        // - 소켓/트랜스폼은 코드에서 다루지 않는다(네가 BP에서 직접 잡는 전제)
        // - 커스텀 콜리전 채널은 BP에서 OverlapTargetChannel / HitBoxObjectType을 골라 넣으면 됨
        LightProfile.Overlap.BoxExtent = FVector(10.f, 25.f, 55.f);         // 기본값(원하면 BP에서 수정)
        LightProfile.Overlap.HitBoxObjectType = ECC_WorldDynamic;           // BP에서 커스텀 Object 채널 선택 가능
        LightProfile.Overlap.OverlapTargetChannel = ECC_Pawn;              // BP에서 네 커스텀 채널로 변경
        LightProfile.Overlap.bHitEachActorOnce = true;
        LightProfile.Overlap.bHitFirstTargetOnly = true;

        // OnHit 효과는 BP에서 구성하거나, 여기서 추가해도 됨
        // 예)
        // FOnHitGameplayEffectSpec DamageSpec;
        // DamageSpec.Effect = UGE_WeaponDamage_Instant::StaticClass();
        // DamageSpec.Level = 1.f;
        // DamageSpec.SetByCallerMagnitudes.Add(FGameplayTag::RequestGameplayTag(TEXT("Data.EnemyDamage"), false), 20.f);
        // LightProfile.OnHitTargetEffects.Add(DamageSpec);

        AttackProfiles.Add(LightAttackTag, LightProfile);
    }
}
