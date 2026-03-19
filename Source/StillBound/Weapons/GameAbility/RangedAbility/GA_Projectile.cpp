// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/GameAbility/RangedAbility/GA_Projectile.h"

#include "Weapons/RangedWeapon/RangedWeaponBase.h"
#include "Weapons/RangedWeapon/ProjectileBase.h"
#include "Weapons/WeaponBase.h"
#include "Character/PlayerCharacter_SB.h"

#include "GameplayTagContainer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

// 수정: RangedWeaponBase의 OnHit spec -> Projectile 전용 spec 변환을
// 수정: Projectile 전용 GA 안으로 내림
namespace
{
    static TArray<FProjectileOnHitGameplayEffectSpec> ConvertToProjectileOnHitSpecs(
        const TArray<FRangedOnHitGameplayEffectSpec>& InSpecs)
    {
        TArray<FProjectileOnHitGameplayEffectSpec> OutSpecs;
        OutSpecs.Reserve(InSpecs.Num());

        for (const FRangedOnHitGameplayEffectSpec& InSpec : InSpecs)
        {
            FProjectileOnHitGameplayEffectSpec OutSpec;
            OutSpec.Effect = InSpec.Effect;
            OutSpec.Level = InSpec.Level;
            OutSpec.SetByCallerMagnitudes = InSpec.SetByCallerMagnitudes;
            OutSpec.Chance = InSpec.Chance;
            OutSpecs.Add(MoveTemp(OutSpec));
        }

        return OutSpecs;
    }
}

UGA_Projectile::UGA_Projectile()
{
    // 몽타주 노티파이 타이밍용 발사 이벤트
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
    // 이 GA를 썼는데 FireProfile이 Hitscan으로 설정돼 있으면 projectile 발사를 막는다.
    if (!CachedProfile.IsProjectileMode())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GA_Projectile] FireProfile is not Projectile mode. Weapon=%s Ability=%s"),
            *GetNameSafe(Weapon),
            *GetNameSafe(this));
        return;
    }

    // 수정: 발사 직전 총구섬광은 그대로 공용 베이스 함수 사용
    SpawnMuzzleFlash(Weapon);

    // 수정: Projectile 전용 발사는 이 GA에서 처리
    FireProjectileOnce(Weapon);
}

void UGA_Projectile::FireProjectileOnce(ARangedWeaponBase* Weapon)
{
    if (!Weapon || !bHasCachedProfile)
    {
        return;
    }

    const FRangedProjectileConfig& Projectile = CachedProfile.Projectile;
    if (!Projectile.IsConfigured())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GA_Projectile] Invalid projectile config. Weapon=%s Ability=%s"),
            *GetNameSafe(Weapon),
            *GetNameSafe(this));
        return;
    }

    UWorld* World = Weapon->GetWorld();
    if (!World)
    {
        return;
    }

    FTransform SpawnTransform;
    FVector ShotDirection;
    if (!TryGetProjectileSpawnTransform(Weapon, SpawnTransform, ShotDirection))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GA_Projectile] TryGetProjectileSpawnTransform failed. Weapon=%s"),
            *GetNameSafe(Weapon));
        return;
    }

    FActorSpawnParameters Params;
    Params.Owner = GetAvatarActorFromActorInfo();
    Params.Instigator = Cast<APawn>(GetAvatarActorFromActorInfo());
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* SpawnedProjectile = World->SpawnActor<AActor>(
        Projectile.ProjectileClass,
        SpawnTransform,
        Params
    );

    if (!SpawnedProjectile)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GA_Projectile] Projectile spawn failed. Class=%s Weapon=%s"),
            *GetNameSafe(Projectile.ProjectileClass),
            *GetNameSafe(Weapon));
        return;
    }

    float FinalDamage = 0.f;
    if (!TryGetCachedFinalDamage(Weapon, FinalDamage))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GA_Projectile] Failed to compute cached final damage. Weapon=%s Ability=%s"),
            *GetNameSafe(Weapon),
            *GetNameSafe(this));
        return;
    }

    // 수정: 무기 프로파일의 OnHit spec을 Projectile 전용 payload로 변환
    const TArray<FProjectileOnHitGameplayEffectSpec> ProjectileOnHitSpecs =
        ConvertToProjectileOnHitSpecs(CachedProfile.OnHitTargetEffects);

    if (AProjectileBase* WeaponProjectile = Cast<AProjectileBase>(SpawnedProjectile))
    {
        WeaponProjectile->InitProjectileData(
            GetAvatarActorFromActorInfo(),
            Weapon,
            BaseDamageEffectClass,
            FinalDamage,
            ProjectileOnHitSpecs
        );
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GA_Projectile] Spawned projectile is not AProjectileBase. Damage/FX init skipped. Projectile=%s"),
            *GetNameSafe(SpawnedProjectile));
    }

    if (!ApplyProjectileLaunchSettings(SpawnedProjectile, ShotDirection))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GA_Projectile] Projectile launch setup failed. Projectile=%s Weapon=%s"),
            *GetNameSafe(SpawnedProjectile),
            *GetNameSafe(Weapon));
            return; // 수정: 발사 세팅 실패 시 소모하지 않음
    }

    // 수정: 실제 발사 성공 후 투척 아이템 1개 소모
    if (APlayerCharacter_SB* PlayerCharacter = Cast<APlayerCharacter_SB>(GetAvatarActorFromActorInfo()))
    {
        const bool bConsumed = PlayerCharacter->ConsumeSelectedThrowableAfterThrow();

        UE_LOG(LogTemp, Log,
            TEXT("[GA_Projectile] ConsumeSelectedThrowableAfterThrow -> %d"),
            bConsumed ? 1 : 0);
    }
}