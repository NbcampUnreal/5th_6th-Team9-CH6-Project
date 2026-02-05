// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/GameAbility/WeaponGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "Engine/EngineTypes.h"
#include "Weapons/MeleeWeaponBase/MeleeWeaponBase.h" 
#include "WeaponMeleeAttackAbilityBase.generated.h"

class AMeleeWeaponBase;
class UAbilityTask_PlayMontageAndWait;

UCLASS(Abstract)
class STILLBOUND_API UWeaponMeleeAttackAbilityBase : public UWeaponGameplayAbility
{
    GENERATED_BODY()

public:
    UWeaponMeleeAttackAbilityBase();

protected:
    /** 무기 프로파일 키 (예: Attack.Light) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee")
    FGameplayTag AttackTag;

    /** 스윕 틱 간격(싱글플레이면 0.01~0.02 추천) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee")
    float SweepInterval = 0.015f;

    // 활성화 중 캐싱
    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

    // 히트 중복 방지
    UPROPERTY()
    TSet<TWeakObjectPtr<AActor>> HitActors;

    // 프로파일 캐시 (struct는 UPROPERTY로 보관 불가하니 멤버로 보관)
    bool bHasCachedProfile = false;
    struct FWeaponAttackProfile CachedProfile;
    struct FMeleeSweepConfig CachedSweep;

protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled
    ) override;

    void StartSweeping();
    void StopSweeping();
    void DoSweepTick();

    bool CacheProfileFromWeapon(AMeleeWeaponBase* Weapon);

    bool ApplyOnHitEffects(AActor* TargetActor);

    FTimerHandle SweepTimerHandle;

    UFUNCTION()
    void OnMontageCompleted();

    UFUNCTION()
    void OnMontageCancelled();

    UFUNCTION()
    void OnMontageInterrupted();

    UFUNCTION()
    void OnMontageBlendOut();
};