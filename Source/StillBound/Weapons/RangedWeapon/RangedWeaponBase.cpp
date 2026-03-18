#include "Weapons/RangedWeapon/RangedWeaponBase.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"

ARangedWeaponBase::ARangedWeaponBase()
{
    StaticWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticWeaponMesh"));
    if (StaticWeaponMesh)
    {
        StaticWeaponMesh->SetupAttachment(GetWeaponRoot());
        StaticWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        StaticWeaponMesh->SetGenerateOverlapEvents(false);
        StaticWeaponMesh->SetCanEverAffectNavigation(false);
    }

    SkeletalWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalWeaponMesh"));
    if (SkeletalWeaponMesh)
    {
        SkeletalWeaponMesh->SetupAttachment(GetWeaponRoot());
        SkeletalWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SkeletalWeaponMesh->SetGenerateOverlapEvents(false);
        SkeletalWeaponMesh->SetCanEverAffectNavigation(false);
    }

    RefreshMeshMode();
}

void ARangedWeaponBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    RefreshMeshMode();
    ValidateFireProfiles();
}

void ARangedWeaponBase::RefreshMeshMode()
{
    const bool bShowSkeletal = bUseSkeletalMesh;

    if (StaticWeaponMesh)
    {
        StaticWeaponMesh->SetHiddenInGame(bShowSkeletal);
        StaticWeaponMesh->SetVisibility(!bShowSkeletal, true);
    }

    if (SkeletalWeaponMesh)
    {
        SkeletalWeaponMesh->SetHiddenInGame(!bShowSkeletal);
        SkeletalWeaponMesh->SetVisibility(bShowSkeletal, true);
    }
}

USceneComponent* ARangedWeaponBase::GetActiveWeaponMesh() const
{
    if (bUseSkeletalMesh)
    {
        return SkeletalWeaponMesh
            ? Cast<USceneComponent>(SkeletalWeaponMesh)
            : Cast<USceneComponent>(StaticWeaponMesh);
    }

    return StaticWeaponMesh
        ? Cast<USceneComponent>(StaticWeaponMesh)
        : Cast<USceneComponent>(SkeletalWeaponMesh);
}

bool ARangedWeaponBase::GetFireProfile(FGameplayTag InputTag, FRangedFireProfile& OutProfile) const
{
    if (!InputTag.IsValid())
    {
        return false;
    }

    const FRangedFireProfile* Found = FireProfiles.Find(InputTag);
    if (!Found)
    {
        return false;
    }

    if (!Found->IsConfigured())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[RangedWeapon] Invalid FireProfile. Weapon=%s InputTag=%s FireMode=%s"),
            *GetNameSafe(this),
            *InputTag.ToString(),
            Found->IsHitscanMode() ? TEXT("Hitscan") : TEXT("Projectile"));
        return false;
    }

    OutProfile = *Found;
    return true;
}

float ARangedWeaponBase::CalculateFinalDamageFromProfile(const FRangedFireProfile& Profile) const
{
    const float BaseDamage = FMath::Max(0.f, GetWeaponDamage());
    const float DamageMultiplier = FMath::Max(0.f, Profile.DamageMultiplier);

    return BaseDamage * DamageMultiplier;
}

bool ARangedWeaponBase::GetFinalDamage(FGameplayTag InputTag, float& OutFinalDamage) const
{
    OutFinalDamage = 0.f;

    FRangedFireProfile Profile;
    if (!GetFireProfile(InputTag, Profile))
    {
        return false;
    }

    OutFinalDamage = CalculateFinalDamageFromProfile(Profile);
    return true;
}

bool ARangedWeaponBase::GetWeaponSocketTransform(FName SocketName, FTransform& OutTransform) const
{
    USceneComponent* ActiveMesh = GetActiveWeaponMesh();
    if (!ActiveMesh)
    {
        return false;
    }

    if (SocketName != NAME_None && ActiveMesh->DoesSocketExist(SocketName))
    {
        OutTransform = ActiveMesh->GetSocketTransform(SocketName, RTS_World);
        return true;
    }

    OutTransform = ActiveMesh->GetComponentTransform();
    return true;
}

void ARangedWeaponBase::ValidateFireProfiles() const
{
    for (const TPair<FGameplayTag, FRangedFireProfile>& Pair : FireProfiles)
    {
        const FGameplayTag& InputTag = Pair.Key;
        const FRangedFireProfile& Profile = Pair.Value;

        if (Profile.IsHitscanMode())
        {
            if (!Profile.Hitscan.IsConfigured())
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("[RangedWeapon] Invalid Hitscan profile. Weapon=%s InputTag=%s MaxDistance=%.2f NumShots=%d"),
                    *GetNameSafe(this),
                    *InputTag.ToString(),
                    Profile.Hitscan.MaxDistance,
                    Profile.Hitscan.NumShots);
            }

            continue;
        }

        if (Profile.IsProjectileMode())
        {
            if (!Profile.Projectile.ProjectileClass)
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("[RangedWeapon] ProjectileClass is null. Weapon=%s InputTag=%s"),
                    *GetNameSafe(this),
                    *InputTag.ToString());
            }

            if (!Profile.Projectile.HasAnyValidLaunchMode())
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("[RangedWeapon] Projectile launch mode invalid. Weapon=%s InputTag=%s bUseInitialSpeed=%d InitialSpeed=%.2f MaxSpeed=%.2f bUseImpulse=%d LaunchImpulse=%.2f"),
                    *GetNameSafe(this),
                    *InputTag.ToString(),
                    Profile.Projectile.bUseInitialSpeed ? 1 : 0,
                    Profile.Projectile.InitialSpeed,
                    Profile.Projectile.MaxSpeed,
                    Profile.Projectile.bUseImpulse ? 1 : 0,
                    Profile.Projectile.LaunchImpulse);
            }
        }
    }
}