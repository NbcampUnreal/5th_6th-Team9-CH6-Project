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

    // GA가 무기에서 프로파일을 가져가는 API
    UFUNCTION(BlueprintCallable, Category = "Weapon|Melee")
    bool GetAttackProfile(FGameplayTag AttackTag, FWeaponAttackProfile& OutProfile) const;

protected:
   
    // 공격 프로파일 테이블: Attack.Light → (몽타주/속도/범위/효과)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee|Data")
    TMap<FGameplayTag, FWeaponAttackProfile> AttackProfiles;


};