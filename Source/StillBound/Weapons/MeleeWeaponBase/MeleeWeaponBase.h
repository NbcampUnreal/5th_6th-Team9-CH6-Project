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
 * OnHit 시 적용할 GE 스펙(부가효과용)
 * - 데미지는 WeaponDamage(=DT) 단일 SoT로 갈 거라,
 *   여기 SetByCaller에 데미지 태그를 넣지 않는 운영을 권장(중복 방지).
 */
USTRUCT(BlueprintType)
struct FOnHitGameplayEffectSpec
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|OnHit")
    TSubclassOf<UGameplayEffect> Effect = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|OnHit")
    float Level = 1.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|OnHit")
    TMap<FGameplayTag, float> SetByCallerMagnitudes;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|OnHit", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Chance = 1.0f;
};

/**
 * 근접 공격 프로파일
 * Key는 InputTag.* (예: InputTag.Attack.Primary)
 */
USTRUCT(BlueprintType)
struct FWeaponAttackProfile
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Attack")
    TObjectPtr<UAnimMontage> Montage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Attack")
    float MontagePlayRate = 1.0f;

    /** 최종 데미지 = WeaponDamage(DT) * DamageMultiplier */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Damage", meta = (ClampMin = "0.0"))
    float DamageMultiplier = 1.0f;

    /** 같은 액터 1회만 타격 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|HitPolicy")
    bool bHitEachActorOnce = true;

    /** 첫 타격 후 히트윈도우 종료 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|HitPolicy")
    bool bHitFirstTargetOnly = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|OnHit")
    TArray<FOnHitGameplayEffectSpec> OnHitTargetEffects;
};

UCLASS(Abstract, Blueprintable)
class STILLBOUND_API AMeleeWeaponBase : public AWeaponBase
{
    GENERATED_BODY()

public:
    AMeleeWeaponBase();

    /** InputTag(예: InputTag.Attack.Primary)로 프로파일 조회 */
    UFUNCTION(BlueprintCallable, Category = "Weapon|Melee")
    bool GetAttackProfile(FGameplayTag InputTag, FWeaponAttackProfile& OutProfile) const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Melee|Mesh")
    UStaticMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

    UFUNCTION(BlueprintCallable, Category = "Weapon|Melee|HitBox")
    UBoxComponent* GetHitBox() const { return HitBox; }

    /** 공격 히트윈도우 동안만 HitBox 활성화 */
    UFUNCTION(BlueprintCallable, Category = "Weapon|Melee|HitBox")
    void SetHitBoxEnabled(bool bEnabled);

protected:
    /** 근접 무기 메쉬(기본은 Static Mesh) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Melee|Mesh", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> WeaponMesh;

    /**
     * 공격 프로파일 맵
     * - Key: InputTag.* (컨트롤러/무기 ActivateByInputTag와 동일)
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee|Data")
    TMap<FGameplayTag, FWeaponAttackProfile> AttackProfiles;

    /** 오버랩 기반 근접 판정 박스(모양/채널/크기는 BP에서 세팅) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Melee|HitBox", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UBoxComponent> HitBox;
};