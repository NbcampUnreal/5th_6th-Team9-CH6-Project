// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/GameAbility/WeaponGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "Engine/EngineTypes.h"              // FHitResult
#include "Weapons/RangedWeapon/RangedWeaponBase.h" // FRangedFireProfile, FRangedHitscanConfig
#include "WeaponRangedAttackAbilityBase.generated.h"

class ARangedWeaponBase;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/**
 * 히트스캔 기반 원거리 공격 GA 베이스
 * - GA가 캐릭터(ActorInfo)에서 카메라 ViewPoint를 직접 참조해서 AimPoint 계산
 * - 2-Trace: (1) 카메라 트레이스 -> AimPoint
 *            (2) 총구(Muzzle) -> AimPoint 방향 트레이스 -> 최종 Hit
 */
UCLASS(Abstract)
class STILLBOUND_API UWeaponRangedAttackAbilityBase : public UWeaponGameplayAbility
{
    GENERATED_BODY()

public:
    UWeaponRangedAttackAbilityBase();

protected:
    /** 어떤 FireProfile을 사용할지 (예: Attack.Primary / Attack.Secondary) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged")
    FGameplayTag FireTag;

    /**
     * 몽타주 타이밍에 맞춰 발사 이벤트를 받을 태그
     * - AnimNotify(또는 NotifyState)에서 SendGameplayEventToActor로 Event.Ranged.Fire 전송
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|FireTiming")
    FGameplayTag FireEventTag;

    /** 몽타주/이벤트가 없을 때 ActivateAbility 즉시 1회 발사 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|FireTiming")
    bool bFireImmediatelyIfNoMontageOrEvent = true;

    /** 디버그 트레이스 표시 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|Debug")
    bool bDebugTrace = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|Debug", meta = (ClampMin = "0.0"))
    float DebugLifeTime = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|Debug", meta = (ClampMin = "0.1"))
    float DebugLineThickness = 1.5f;

protected:
    // runtime
    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitGameplayEvent> FireEventTask;

    bool bHasCachedProfile = false;

    UPROPERTY()
    FRangedFireProfile CachedProfile;

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

protected:
    bool CacheFireProfileFromWeapon(ARangedWeaponBase* Weapon);

    /** ActorInfo 기반 ViewPoint(카메라/컨트롤러 시점) 획득 */
    bool GetViewPoint(FVector& OutLoc, FRotator& OutRot) const;

    /** 트레이스에서 무시할 액터(자기/무기) 등록 */
    void BuildTraceParams(FCollisionQueryParams& OutParams, bool bTraceComplex) const;

    /** LineTrace 또는 SphereTrace(반경>0) 1회 수행 */
    bool TraceSingle(
        const FVector& Start,
        const FVector& End,
        const FRangedHitscanConfig& Hitscan,
        FHitResult& OutHit
    ) const;

    /** 2-Trace로 최종 히트 계산 (ViewTrace + MuzzleTrace) */
    bool ComputeFinalHitscanHit(
        ARangedWeaponBase* Weapon,
        const FRangedHitscanConfig& Hitscan,
        FHitResult& OutFinalHit,
        FVector& OutAimPoint
    ) const;

    /** 한 번의 발사(NumShots만큼 반복) */
    void FireHitscanOnce(ARangedWeaponBase* Weapon);

    /** 히트 대상에게 BaseDamage + OnHitTargetEffects 적용 */
    bool ApplyRangedOnHitEffects(AActor* TargetActor) const;

protected:
    // AbilityTask callbacks
    UFUNCTION()
    void OnFireEventReceived(FGameplayEventData Payload);

    UFUNCTION()
    void OnMontageCompleted();

    UFUNCTION()
    void OnMontageCancelled();

    UFUNCTION()
    void OnMontageInterrupted();

    UFUNCTION()
    void OnMontageBlendOut();
};
