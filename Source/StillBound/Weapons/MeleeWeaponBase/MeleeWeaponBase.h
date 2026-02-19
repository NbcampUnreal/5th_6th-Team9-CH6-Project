#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagContainer.h"
#include "MeleeWeaponBase.generated.h"

class UAnimMontage;
class UGameplayEffect;
class UBoxComponent;
class UStaticMeshComponent;
/**
 * 근접 무기 오버랩(히트박스) 설정
 * - 소켓/트랜스폼은 코드에서 건드리지 않는다. (BP에서 직접 잡는 전제)
 * - BP에서 커스텀 콜리전 채널을 선택할 수 있게 노출한다.
 */
USTRUCT(BlueprintType)
struct FMeleeOverlapConfig
{
    GENERATED_BODY()

    /** 히트박스 크기(Extent). 필요 없으면 BP에서 HitBox 컴포넌트로 직접 조절하고, GA에서 ApplyOverlapConfig 안 불러도 됨 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Overlap")
    FVector BoxExtent = FVector(8.f, 20.f, 50.f);

    /** 히트박스의 ObjectType (필요 시 커스텀 Object Channel 선택 가능) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Overlap")
    TEnumAsByte<ECollisionChannel> HitBoxObjectType = ECC_WorldDynamic;

    /** 오버랩 대상 채널(= 상대의 ObjectType 채널). 커스텀 채널을 BP에서 선택 가능 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Overlap")
    TEnumAsByte<ECollisionChannel> OverlapTargetChannel = ECC_Pawn;

    /** 같은 액터를 1회만 타격 (실제 필터링은 GA에서 사용) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Overlap")
    bool bHitEachActorOnce = true;

    /** 첫 타격만 인정(히트 후 히트박스를 바로 끔) (실제 처리는 GA에서) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Overlap")
    bool bHitFirstTargetOnly = true;
};

USTRUCT(BlueprintType)
struct FOnHitGameplayEffectSpec
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|OnHit")
    TSubclassOf<UGameplayEffect> Effect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|OnHit")
    float Level = 1.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|OnHit")
    TMap<FGameplayTag, float> SetByCallerMagnitudes;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|OnHit", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Chance = 1.0f;
};

USTRUCT(BlueprintType)
struct FWeaponAttackProfile
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Attack")
    TObjectPtr<UAnimMontage> Montage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Attack")
    float MontagePlayRate = 1.0f;

    /** ? 오버랩 기반(소켓/트랜스폼은 건드리지 않음) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Attack")
    FMeleeOverlapConfig Overlap;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Attack")
    TArray<FOnHitGameplayEffectSpec> OnHitTargetEffects;
};

UCLASS(Abstract, Blueprintable)
class STILLBOUND_API AMeleeWeaponBase : public AWeaponBase
{
    GENERATED_BODY()

public:
    AMeleeWeaponBase();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Melee")
    bool GetAttackProfile(FGameplayTag AttackTag, FWeaponAttackProfile& OutProfile) const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Melee|Mesh")
    UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

    UFUNCTION(BlueprintCallable, Category = "Weapon|Melee|Overlap")
    UBoxComponent* GetHitBox() const { return HitBox; }

    /** 프로파일의 Overlap 설정을 히트박스에 반영(크기/채널만). 트랜스폼은 건드리지 않음 */
    UFUNCTION(BlueprintCallable, Category = "Weapon|Melee|Overlap")
    void ApplyOverlapConfig(const FMeleeOverlapConfig& Config);

    /** 공격 중에만 히트박스 On/Off */
    UFUNCTION(BlueprintCallable, Category = "Weapon|Melee|Overlap")
    void SetHitBoxEnabled(bool bEnabled);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Melee|Mesh", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> WeaponMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee|Data")
    TMap<FGameplayTag, FWeaponAttackProfile> AttackProfiles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Melee|Overlap", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UBoxComponent> HitBox;
};
