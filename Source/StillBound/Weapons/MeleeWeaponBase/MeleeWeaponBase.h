// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagContainer.h"
#include "MeleeWeaponBase.generated.h"

class UAnimMontage;
class UGameplayEffect;
class UGameplayAbility;
class UAbilitySystemComponent;
USTRUCT(BlueprintType)
struct FMeleeSweepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Sweep")
    FName TraceStartSocket = TEXT("TraceStart");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Sweep")
    FName TraceEndSocket = TEXT("TraceEnd");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Sweep")
    float MaxDistance = 180.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Sweep")
    float Radius = 14.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Sweep")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Sweep")
    bool bHitEachActorOnce = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Sweep")
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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Attack")
    FMeleeSweepConfig Sweep;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Attack")
    TArray<FOnHitGameplayEffectSpec> OnHitTargetEffects;
};

/**
 * 근접 무기 베이스(스윕 방식)
 * - “GA 실행”과 “프로파일 데이터 제공”까지 담당
 * - 실제 공격 실행(몽타주/스윕/OnHit 적용)은 GA가 담당
 */
UCLASS(Abstract, Blueprintable)
class STILLBOUND_API AMeleeWeaponBase : public AWeaponBase
{
    GENERATED_BODY()

public:
    AMeleeWeaponBase();

    // GA 연결: 장착 시점에 GrantedAbilities(입력태그->GA) 구성이 누락되면 자동 보정
    virtual void Equip(AActor* NewOwner, UAbilitySystemComponent* InASC) override;

    // 입력에서 호출할 편의 함수(Primary)
    UFUNCTION(BlueprintCallable, Category = "Weapon|Melee")
    bool RequestPrimaryAttack();

    // GA가 무기에서 프로파일을 가져가는 API
    UFUNCTION(BlueprintCallable, Category = "Weapon|Melee")
    bool GetAttackProfile(FGameplayTag AttackTag, FWeaponAttackProfile& OutProfile) const;

protected:
    // 어떤 InputTag가 “Primary 공격”인지
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee|Input")
    FGameplayTag PrimaryAttackInputTag;

    // Primary 입력에서 어떤 GA를 실행할지(예: UGA_MeleeLight)
    // - Club/Sword/Dagger에서 바꿔도 되고, BP에서 설정해도 됨
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee|GAS")
    TSubclassOf<UGameplayAbility> PrimaryAttackAbilityClass;

    // 공격 프로파일 테이블: Attack.Light → (몽타주/속도/범위/효과)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee|Data")
    TMap<FGameplayTag, FWeaponAttackProfile> AttackProfiles;

protected:
    void EnsurePrimaryAttackGrant();
};