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

/** Hitscan(Trace) 설정: Hitscan GA가 사용 */
USTRUCT(BlueprintType)
struct FRangedHitscanConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Hitscan")
    FName MuzzleSocket = TEXT("Muzzle");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Hitscan", meta = (ClampMin = "0.0"))
    float MaxDistance = 10000.f;

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

/** Projectile 설정: Projectile GA가 사용 */
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

/** OnHit 부가효과: GA가 적용 */
USTRUCT(BlueprintType)
struct FRangedOnHitGameplayEffectSpec
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|OnHit")
    TSubclassOf<UGameplayEffect> Effect = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|OnHit")
    float Level = 1.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|OnHit")
    TMap<FGameplayTag, float> SetByCallerMagnitudes;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|OnHit", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Chance = 1.0f;
};

/** 탄약/연사 기본 데이터 */
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
 * 발사 프로파일
 * - Key는 InputTag.* (예: InputTag.Attack.Primary)
 * - 최종 데미지 = WeaponDamage(DT) * DamageMultiplier
 */
USTRUCT(BlueprintType)
struct FRangedFireProfile
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Fire")
    TObjectPtr<UAnimMontage> Montage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Fire")
    float MontagePlayRate = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Damage", meta = (ClampMin = "0.0"))
    float DamageMultiplier = 1.0f;

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

    UFUNCTION(BlueprintCallable, Category = "Weapon|Ranged")
    bool GetFireProfile(FGameplayTag InputTag, FRangedFireProfile& OutProfile) const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Ranged|Mesh")
    USceneComponent* GetActiveWeaponMesh() const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Ranged|Mesh")
    UStaticMeshComponent* GetStaticWeaponMesh() const { return StaticWeaponMesh; }

    UFUNCTION(BlueprintCallable, Category = "Weapon|Ranged|Mesh")
    USkeletalMeshComponent* GetSkeletalWeaponMesh() const { return SkeletalWeaponMesh; }

    UFUNCTION(BlueprintCallable, Category = "Weapon|Ranged")
    bool GetWeaponSocketTransform(FName SocketName, FTransform& OutTransform) const;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|Mesh")
    bool bUseSkeletalMesh = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Mesh", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> StaticWeaponMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged|Mesh", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USkeletalMeshComponent> SkeletalWeaponMesh;

    /** Key = InputTag.* */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|Data")
    TMap<FGameplayTag, FRangedFireProfile> FireProfiles;

protected:
    void RefreshMeshMode();
    virtual void OnConstruction(const FTransform& Transform) override;
};