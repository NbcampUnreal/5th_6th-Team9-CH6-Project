#include "Weapons/RangedWeapon/RangedWeaponBase.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"

ARangedWeaponBase::ARangedWeaponBase()
{
    // WeaponBase에 Root만 있으므로, 파생 클래스에서 시각 컴포넌트를 만든다.

    StaticWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticWeaponMesh"));
    if (StaticWeaponMesh)
    {
        StaticWeaponMesh->SetupAttachment(GetWeaponRoot());
        StaticWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        StaticWeaponMesh->SetGenerateOverlapEvents(false);
    }

    SkeletalWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalWeaponMesh"));
    if (SkeletalWeaponMesh)
    {
        SkeletalWeaponMesh->SetupAttachment(GetWeaponRoot());
        SkeletalWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SkeletalWeaponMesh->SetGenerateOverlapEvents(false);
    }

    RefreshMeshMode();
}

void ARangedWeaponBase::RefreshMeshMode()
{
    // 선택된 메쉬만 보이게(나머지는 숨김). 둘 다 만들어두면 BP에서 에셋만 바꿔 끼우기 쉬움.
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
        return SkeletalWeaponMesh ? Cast<USceneComponent>(SkeletalWeaponMesh) : Cast<USceneComponent>(StaticWeaponMesh);
    }
    return StaticWeaponMesh ? Cast<USceneComponent>(StaticWeaponMesh) : Cast<USceneComponent>(SkeletalWeaponMesh);
}

bool ARangedWeaponBase::GetFireProfile(FGameplayTag FireTag, FRangedFireProfile& OutProfile) const
{
    if (!FireTag.IsValid())
    {
        return false;
    }

    const FRangedFireProfile* Found = FireProfiles.Find(FireTag);
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
