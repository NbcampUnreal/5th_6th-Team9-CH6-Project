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
        return SkeletalWeaponMesh ? Cast<USceneComponent>(SkeletalWeaponMesh)
            : Cast<USceneComponent>(StaticWeaponMesh);
    }

    return StaticWeaponMesh ? Cast<USceneComponent>(StaticWeaponMesh)
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

    OutProfile = *Found;
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