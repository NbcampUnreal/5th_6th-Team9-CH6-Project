#include "Weapons/MeleeWeaponBase/MeleeWeaponBase.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h" 

AMeleeWeaponBase::AMeleeWeaponBase()
{
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    if (WeaponMesh)
    {
        // WeaponBase의 Root(씬 컴포넌트)에 부착
        WeaponMesh->SetupAttachment(GetWeaponRoot());

        // 근접 무기는 보통 메쉬 자체 콜리전을 쓰지 않고 히트박스/트레이스로 처리
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        WeaponMesh->SetGenerateOverlapEvents(false);
    }

    HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("MeleeHitBox"));
    if (HitBox)
    {
        HitBox->SetupAttachment(WeaponMesh);

        // 기본값(필요하면 BP에서 직접 수정해도 됨)
        HitBox->SetBoxExtent(FVector(8.f, 20.f, 50.f));

        // 트랜스폼(위치/회전)은 네가 BP에서 직접 잡는 전제 → 여기서 건드리지 않음

        // 기본은 꺼둠(공격 중에만 켬)
        HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        HitBox->SetGenerateOverlapEvents(false);

        // 기본 충돌 응답(ApplyOverlapConfig로 덮어씌울 수 있음)
        HitBox->SetCollisionObjectType(ECC_WorldDynamic);
        HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
        HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

        HitBox->SetCanEverAffectNavigation(false);
    }
}

bool AMeleeWeaponBase::GetAttackProfile(FGameplayTag AttackTag, FWeaponAttackProfile& OutProfile) const
{
    if (!AttackTag.IsValid())
    {
        return false;
    }

    const FWeaponAttackProfile* Found = AttackProfiles.Find(AttackTag);
    if (!Found)
    {
        return false;
    }

    OutProfile = *Found;
    return true;
}

void AMeleeWeaponBase::ApplyOverlapConfig(const FMeleeOverlapConfig& Config)
{
    if (!HitBox)
    {
        return;
    }

    // ? 트랜스폼은 건드리지 않는다 (BP에서 직접 잡는 전제)
    HitBox->SetBoxExtent(Config.BoxExtent);

    // ? BP에서 만든 커스텀 채널 선택 가능
    HitBox->SetCollisionObjectType(Config.HitBoxObjectType);
    HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    HitBox->SetCollisionResponseToChannel(Config.OverlapTargetChannel, ECR_Overlap);
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

        // 켠 프레임에 이미 겹쳐있는 대상도 잡고 싶으면 유지(원치 않으면 주석 처리 가능)
        HitBox->UpdateOverlaps();
    }
    else
    {
        HitBox->SetGenerateOverlapEvents(false);
        HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}
