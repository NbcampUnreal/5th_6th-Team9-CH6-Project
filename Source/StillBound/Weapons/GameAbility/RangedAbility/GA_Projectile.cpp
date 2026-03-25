// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/GameAbility/RangedAbility/GA_Projectile.h"

#include "Weapons/RangedWeapon/RangedWeaponBase.h"
#include "Weapons/RangedWeapon/ProjectileBase.h"
#include "Weapons/WeaponBase.h"
#include "Character/PlayerCharacter_SB.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

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

    static FProjectileImpactFXPayload ConvertToProjectileImpactFXPayload(const AWeaponBase* Weapon)
    {
        FProjectileImpactFXPayload OutPayload;

        if (!Weapon)
        {
            return OutPayload;
        }

        const FWeaponHitImpactFX& WeaponFX = Weapon->GetHitImpactFX();
        OutPayload.NiagaraSystem = WeaponFX.NiagaraSystem;
        OutPayload.Scale = WeaponFX.Scale;
        OutPayload.LocationOffset = WeaponFX.LocationOffset;
        OutPayload.RotationOffset = WeaponFX.RotationOffset;
        OutPayload.bUseImpactNormalRotation = WeaponFX.bUseImpactNormalRotation;

        return OutPayload;
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

    // 이 GA인데 FireProfile이 Hitscan이면 projectile 발사를 막는다.
    if (!CachedProfile.IsProjectileMode())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GA_Projectile] FireProfile is not Projectile mode. Weapon=%s Ability=%s"),
            *GetNameSafe(Weapon),
            *GetNameSafe(this));
        return;
    }

    // 발사 직전 총구섬광
    SpawnMuzzleFlash(Weapon);

    // Projectile 전용 발사 처리
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

    float FinalDamage = 0.f;
    if (!TryGetCachedFinalDamage(Weapon, FinalDamage))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GA_Projectile] Failed to compute cached final damage. Weapon=%s Ability=%s"),
            *GetNameSafe(Weapon),
            *GetNameSafe(this));
        return;
    }

    const TArray<FProjectileOnHitGameplayEffectSpec> ProjectileOnHitSpecs =
        ConvertToProjectileOnHitSpecs(CachedProfile.OnHitTargetEffects);

    const FProjectileImpactFXPayload ImpactFXPayload =
        ConvertToProjectileImpactFXPayload(Weapon);

    FActorSpawnParameters Params;
    Params.Owner = GetAvatarActorFromActorInfo();
    Params.Instigator = Cast<APawn>(GetAvatarActorFromActorInfo());
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* SpawnedActor = World->SpawnActor<AActor>(
        Projectile.ProjectileClass,
        SpawnTransform,
        Params
    );

    if (!SpawnedActor)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GA_Projectile] Projectile spawn failed. Class=%s Weapon=%s"),
            *GetNameSafe(Projectile.ProjectileClass),
            *GetNameSafe(Weapon));
        return;
    }


    AProjectileBase* SpawnedProjectile = Cast<AProjectileBase>(SpawnedActor);
    if (!SpawnedProjectile)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GA_Projectile] Spawned actor is not AProjectileBase. Class=%s Actor=%s"),
            *GetNameSafe(Projectile.ProjectileClass),
            *GetNameSafe(SpawnedActor));

        SpawnedActor->Destroy();
        return;
    }

    UAbilitySystemComponent* SourceASC =
        CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr;

    SpawnedProjectile->InitProjectileData(
        GetAvatarActorFromActorInfo(),
        SourceASC,
        BaseDamageEffectClass,
        FinalDamage,
        ProjectileOnHitSpecs,
        ImpactFXPayload
    );

    if (!ApplyProjectileLaunchSettings(SpawnedProjectile, ShotDirection))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GA_Projectile] Projectile launch setup failed. Projectile=%s Weapon=%s"),
            *GetNameSafe(SpawnedProjectile),
            *GetNameSafe(Weapon));

        SpawnedProjectile->Destroy(); // 수정: 발사 세팅 실패 시 남겨두지 않음
        return;
    }

    // 실제 발사 성공 후 투척 아이템 1개 소모
    if (APlayerCharacter_SB* PlayerCharacter = Cast<APlayerCharacter_SB>(GetAvatarActorFromActorInfo()))
    {
        const bool bConsumed = PlayerCharacter->ConsumeSelectedThrowableAfterThrow();

        UE_LOG(LogTemp, Log,
            TEXT("[GA_Projectile] ConsumeSelectedThrowableAfterThrow -> %d"),
            bConsumed ? 1 : 0);
    }
}