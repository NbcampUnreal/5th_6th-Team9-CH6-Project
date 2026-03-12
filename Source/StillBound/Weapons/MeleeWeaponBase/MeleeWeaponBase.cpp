#include "Weapons/MeleeWeaponBase/MeleeWeaponBase.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/EngineTypes.h"

AMeleeWeaponBase::AMeleeWeaponBase()
{
    // Mesh
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    if (WeaponMesh)
    {
        WeaponMesh->SetupAttachment(GetWeaponRoot());
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        WeaponMesh->SetGenerateOverlapEvents(false);
        WeaponMesh->SetCanEverAffectNavigation(false);
    }

    // HitBox (SoT: BP에서 모양/채널/크기 조절)
    HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("MeleeHitBox"));
    if (HitBox)
    {
        HitBox->SetupAttachment(WeaponMesh);

        // 기본값은 "꺼짐" (히트윈도우에서만 켬)
        HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        HitBox->SetGenerateOverlapEvents(false);

        // 기본 오버랩 채널(필요하면 BP에서 변경)
        HitBox->SetCollisionObjectType(ECC_WorldDynamic);
        HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
        HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

        HitBox->SetCanEverAffectNavigation(false);
    }
}

bool AMeleeWeaponBase::GetAttackProfile(FGameplayTag InputTag, FWeaponAttackProfile& OutProfile) const
{
    if (!InputTag.IsValid())
    {
        return false;
    }

    const FWeaponAttackProfile* Found = AttackProfiles.Find(InputTag);
    if (!Found)
    {
        return false;
    }

    OutProfile = *Found;
    return true;
}

void AMeleeWeaponBase::SetHitBoxEnabled(bool bEnabled)
{
    if (!HitBox)
    {
        return;
    }

    if (bEnabled)
    {
        HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        HitBox->SetGenerateOverlapEvents(true);
        HitBox->UpdateOverlaps();
    }
    else
    {
        HitBox->SetGenerateOverlapEvents(false);
        HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}