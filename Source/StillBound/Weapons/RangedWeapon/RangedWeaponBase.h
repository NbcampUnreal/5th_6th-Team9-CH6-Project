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
class UNiagaraSystem;
 

UENUM(BlueprintType)
enum class ERangedFireMode : uint8
{
    Hitscan    UMETA(DisplayName = "Hitscan"),
    Projectile UMETA(DisplayName = "Projectile")
};

/** Hitscan(Trace) 설정 */
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

    bool IsConfigured() const
    {
        return MaxDistance > 0.f && NumShots > 0;
    }
};

/** Projectile 설정 */
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

    // 속도 방식
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile|Launch")
    bool bUseInitialSpeed = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile|Launch", meta = (ClampMin = "0.0"))
    float InitialSpeed = 3000.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile|Launch", meta = (ClampMin = "0.0"))
    float MaxSpeed = 3000.f;

    // 임펄스 방식
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile|Launch")
    bool bUseImpulse = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile|Launch", meta = (ClampMin = "0.0"))
    float LaunchImpulse = 0.f;

    // 수정:
    // float GravityScale 제거
    // 이제 중력은 ON/OFF만 사용
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile|Launch")
    bool bEnableGravity = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile|Launch", meta = (ClampMin = "0.0"))
    float LifeSeconds = 5.f;

    bool HasValidSpeedMode() const
    {
        return bUseInitialSpeed && InitialSpeed > 0.f && MaxSpeed > 0.f;
    }

    bool HasValidImpulseMode() const
    {
        return bUseImpulse && LaunchImpulse > 0.f;
    }

    bool HasAnyValidLaunchMode() const
    {
        return HasValidSpeedMode() || HasValidImpulseMode();
    }

    bool IsConfigured() const
    {
        return ProjectileClass != nullptr && HasAnyValidLaunchMode();
    }
};

/** OnHit 부가효과 */
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

/** 총구섬광 FX 설정 */
USTRUCT(BlueprintType)
struct FRangedMuzzleFlashConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|FX|MuzzleFlash")
    TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|FX|MuzzleFlash")
    FVector LocationOffset = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|FX|MuzzleFlash")
    FRotator RotationOffset = FRotator::ZeroRotator;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|FX|MuzzleFlash")
    FVector Scale = FVector(1.f, 1.f, 1.f);

    bool IsConfigured() const
    {
        return NiagaraSystem != nullptr;
    }
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
    TArray<FRangedOnHitGameplayEffectSpec> OnHitTargetEffects;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Fire")
    ERangedFireMode FireMode = ERangedFireMode::Hitscan;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Fire")
    FRangedHitscanConfig Hitscan;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Fire")
    FRangedProjectileConfig Projectile;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|FX|MuzzleFlash")
    FRangedMuzzleFlashConfig MuzzleFlash;

    bool IsHitscanMode() const
    {
        return FireMode == ERangedFireMode::Hitscan;
    }

    bool IsProjectileMode() const
    {
        return FireMode == ERangedFireMode::Projectile;
    }

    FName GetMuzzleSocketName() const
    {
        return IsProjectileMode() ? Projectile.MuzzleSocket : Hitscan.MuzzleSocket;
    }

    bool HasMuzzleFlash() const
    {
        return MuzzleFlash.IsConfigured();
    }

    bool IsConfigured() const
    {
        return IsHitscanMode() ? Hitscan.IsConfigured() : Projectile.IsConfigured();
    }
};

UCLASS(Abstract, Blueprintable)
class STILLBOUND_API ARangedWeaponBase : public AWeaponBase
{
    GENERATED_BODY()

public:
    ARangedWeaponBase();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Ranged")
    bool GetFireProfile(FGameplayTag InputTag, FRangedFireProfile& OutProfile) const;

    UFUNCTION(BlueprintPure, Category = "Weapon|Ranged|Damage")
    float CalculateFinalDamageFromProfile(const FRangedFireProfile& Profile) const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Ranged|Damage")
    bool GetFinalDamage(FGameplayTag InputTag, float& OutFinalDamage) const;

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
    void ValidateFireProfiles() const;

    virtual void OnConstruction(const FTransform& Transform) override;
};