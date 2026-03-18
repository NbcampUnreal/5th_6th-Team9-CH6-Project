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
class AProjectileBase;
class UNiagaraSystem;

//프로파일이 어떤 발사 방식을 쓰는지 명시
UENUM(BlueprintType)
enum class ERangedFireMode : uint8
{
    Hitscan    UMETA(DisplayName = "Hitscan"),
    Projectile UMETA(DisplayName = "Projectile")
};

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

    // //추가: Hitscan 기본 유효성 체크
    bool IsConfigured() const
    {
        return MaxDistance > 0.f && NumShots > 0;
    }

};

/** Projectile 설정: Projectile GA가 사용 */
USTRUCT(BlueprintType)
struct FRangedProjectileConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile")
    TSubclassOf<AProjectileBase> ProjectileClass = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile")
    FName MuzzleSocket = TEXT("Muzzle");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile")
    FVector SpawnOffset = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile")
    bool bUseControllerViewRotation = true;

    // //추가: 속도 방식
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile|Launch")
    bool bUseInitialSpeed = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile|Launch", meta = (ClampMin = "0.0"))
    float InitialSpeed = 3000.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile|Launch", meta = (ClampMin = "0.0"))
    float MaxSpeed = 3000.f;

    // //추가: 임펄스 방식
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile|Launch")
    bool bUseImpulse = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile|Launch", meta = (ClampMin = "0.0"))
    float LaunchImpulse = 0.f;

    // //추가: 공통 이동 설정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile|Launch")
    float GravityScale = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Projectile|Launch", meta = (ClampMin = "0.0"))
    float LifeSeconds = 5.f;

    // //추가: Speed 유효성
    bool HasValidSpeedMode() const
    {
        return bUseInitialSpeed && InitialSpeed > 0.f && MaxSpeed > 0.f;
    }

    // //추가: Impulse 유효성
    bool HasValidImpulseMode() const
    {
        return bUseImpulse && LaunchImpulse > 0.f;
    }

    // //추가: 둘 중 하나는 최소 있어야 함
    bool HasAnyValidLaunchMode() const
    {
        return HasValidSpeedMode() || HasValidImpulseMode();
    }

    // //추가: Projectile 전체 유효성
    bool IsConfigured() const
    {
        return ProjectileClass != nullptr && HasAnyValidLaunchMode();
    }
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

USTRUCT(BlueprintType)
struct FRangedMuzzleFlashConfig
{
    GENERATED_BODY()

    // 총구섬광 추가: 발사 순간 재생할 나이아가라
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|FX|MuzzleFlash")
    TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

    // 총구섬광 추가: 총구 소켓 기준 위치 보정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|FX|MuzzleFlash")
    FVector LocationOffset = FVector::ZeroVector;

    // 총구섬광 추가: 총구 소켓 기준 회전 보정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|FX|MuzzleFlash")
    FRotator RotationOffset = FRotator::ZeroRotator;

    // 총구섬광 추가: FX 스케일
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|FX|MuzzleFlash")
    FVector Scale = FVector(1.f, 1.f, 1.f);

    bool IsConfigured() const
    {
        return NiagaraSystem != nullptr;
    }
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

    //이 프로파일이 어떤 발사 방식을 쓰는지
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Fire")
    ERangedFireMode FireMode = ERangedFireMode::Hitscan;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Fire")
    FRangedHitscanConfig Hitscan;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|Fire")
    FRangedProjectileConfig Projectile;

    // 총구섬광 추가: 발사 순간 FX 데이터
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranged|FX|MuzzleFlash")
    FRangedMuzzleFlashConfig MuzzleFlash;

    // //추가
    bool IsHitscanMode() const
    {
        return FireMode == ERangedFireMode::Hitscan;
    }

    // //추가
    bool IsProjectileMode() const
    {
        return FireMode == ERangedFireMode::Projectile;
    }

    // 총구섬광 추가: 현재 FireMode 기준 총구 소켓 이름 반환
    FName GetMuzzleSocketName() const
    {
        return IsProjectileMode() ? Projectile.MuzzleSocket : Hitscan.MuzzleSocket;
    }

    // 총구섬광 추가: 총구섬광 설정 여부
    bool HasMuzzleFlash() const
    {
        return MuzzleFlash.IsConfigured();
    }

    // //추가: 현재 선택된 FireMode 기준으로만 검증
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

    //추가: 현재 FireProfiles 세팅 검증
    void ValidateFireProfiles() const;

    virtual void OnConstruction(const FTransform& Transform) override;
};