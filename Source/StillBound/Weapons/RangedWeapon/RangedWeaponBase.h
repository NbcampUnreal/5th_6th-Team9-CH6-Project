#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagContainer.h"
#include "RangedWeaponBase.generated.h"

class UAnimMontage;
class UGameplayEffect;
class AActor;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class USceneComponent;

/** 히트스캔(Trace) 데이터: Hitscan GA가 사용 */
USTRUCT(BlueprintType)
struct FRangedHitscanConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Hitscan")
    FName MuzzleSocket = TEXT("Muzzle");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Hitscan", meta = (ClampMin = "0.0"))
    float MaxDistance = 10000.f;

    /** 0이면 LineTrace, 0보다 크면 SphereTrace 반경 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Hitscan", meta = (ClampMin = "0.0"))
    float Radius = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Hitscan")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Hitscan")
    bool bTraceComplex = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Hitscan", meta = (ClampMin = "1"))
    int32 NumShots = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Hitscan", meta = (ClampMin = "0.0"))
    float SpreadHalfAngleDeg = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Hitscan")
    bool bUseControllerViewRotation = true;
};

/** 프로젝타일 데이터: Projectile GA가 사용 */
USTRUCT(BlueprintType)
struct FRangedProjectileConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile")
    TSubclassOf<AActor> ProjectileClass = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile")
    FName MuzzleSocket = TEXT("Muzzle");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile")
    FVector SpawnOffset = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile")
    bool bUseControllerViewRotation = true;
};

/** OnHit 효과: GA가 적용 */
USTRUCT(BlueprintType)
struct FRangedOnHitGameplayEffectSpec
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|OnHit")
    TSubclassOf<UGameplayEffect> Effect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|OnHit")
    float Level = 1.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|OnHit")
    TMap<FGameplayTag, float> SetByCallerMagnitudes;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|OnHit", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Chance = 1.0f;
};

/** 탄약/연사 “기본 데이터” (상태/소모/리로드 로직은 Ranged GA Base에서 처리) */
USTRUCT(BlueprintType)
struct FRangedAmmoConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Ammo", meta = (ClampMin = "0"))
    int32 MagazineSize = 30;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Ammo", meta = (ClampMin = "0"))
    int32 MaxReserveAmmo = 90;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Ammo", meta = (ClampMin = "1"))
    int32 AmmoPerShot = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Timing", meta = (ClampMin = "0.0"))
    float FireInterval = 0.12f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Timing", meta = (ClampMin = "0.0"))
    float ReloadDuration = 1.6f;
};

/**
 * 발사 프로파일(= 무기 데이터)
 * - Hitscan GA는 Hitscan만 사용
 * - Projectile GA는 Projectile만 사용
 * - 타입 구분(enum) 없음: “어떤 GA를 부여했는지”가 타입(SoT)
 */
USTRUCT(BlueprintType)
struct FRangedFireProfile
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Fire")
    TObjectPtr<UAnimMontage> Montage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Fire")
    float MontagePlayRate = 1.0f;

    /** 참고용 기본 데미지(권장: GA에서 Damage GE + SetByCaller로 적용) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Damage", meta = (ClampMin = "0.0"))
    float BaseDamage = 10.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Fire")
    FRangedAmmoConfig Ammo;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Fire")
    TArray<FRangedOnHitGameplayEffectSpec> OnHitTargetEffects;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Fire")
    FRangedHitscanConfig Hitscan;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Fire")
    FRangedProjectileConfig Projectile;
};

UCLASS(Abstract, Blueprintable)
class STILLBOUND_API ARangedWeaponBase : public AWeaponBase
{
    GENERATED_BODY()

public:
    ARangedWeaponBase();

    /** FireTag(예: Attack.Primary / Attack.Secondary)로 프로파일 조회 */
    UFUNCTION(BlueprintCallable, Category = "Weapon|Ranged")
    bool GetFireProfile(FGameplayTag FireTag, FRangedFireProfile& OutProfile) const;

    /** 현재 선택된(활성) 무기 메쉬 컴포넌트(Static 또는 Skeletal) */
    UFUNCTION(BlueprintCallable, Category = "Weapon|Ranged|Mesh")
    USceneComponent* GetActiveWeaponMesh() const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Ranged|Mesh")
    UStaticMeshComponent* GetStaticWeaponMesh() const { return StaticWeaponMesh; }

    UFUNCTION(BlueprintCallable, Category = "Weapon|Ranged|Mesh")
    USkeletalMeshComponent* GetSkeletalWeaponMesh() const { return SkeletalWeaponMesh; }

    /** 소켓 월드 트랜스폼 유틸(발사용/이펙트용). 없으면 메쉬 트랜스폼 반환 */
    UFUNCTION(BlueprintCallable, Category = "Weapon|Ranged")
    bool GetWeaponSocketTransform(FName SocketName, FTransform& OutTransform) const;

protected:
    /** 애님/리코일/장전까지 염두: 기본값 true */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|Mesh")
    bool bUseSkeletalMesh = true;

    /** 스태틱 메쉬(애님 없는 에셋 대응) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Mesh", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> StaticWeaponMesh;

    /** 스켈레탈 메쉬(총기/장전/리코일/소켓) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Mesh", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USkeletalMeshComponent> SkeletalWeaponMesh;

    /** FireTag -> 프로파일 데이터 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|Data")
    TMap<FGameplayTag, FRangedFireProfile> FireProfiles;

protected:
    /** 에디터/런타임에서 메쉬 모드 반영(가시성/충돌) */
    void RefreshMeshMode();
};
