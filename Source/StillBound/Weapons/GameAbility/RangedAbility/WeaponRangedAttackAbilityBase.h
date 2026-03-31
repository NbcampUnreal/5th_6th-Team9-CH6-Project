#pragma once

#include "CoreMinimal.h"
#include "Weapons/GameAbility/WeaponGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "Engine/EngineTypes.h"
#include "Weapons/RangedWeapon/RangedWeaponBase.h"
#include "WeaponRangedAttackAbilityBase.generated.h"

class ARangedWeaponBase;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

UCLASS(Abstract)
class STILLBOUND_API UWeaponRangedAttackAbilityBase : public UWeaponGameplayAbility
{
    GENERATED_BODY()

public:
    UWeaponRangedAttackAbilityBase();

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|FireTiming")
    FGameplayTag FireEventTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|FireTiming")
    bool bFireImmediatelyIfNoMontageOrEvent = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|Debug")
    bool bDebugTrace = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|Debug", meta = (ClampMin = "0.0"))
    float DebugLifeTime = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|Debug", meta = (ClampMin = "0.1"))
    float DebugLineThickness = 1.5f;

    // [추가] 카메라 trace 시작점을 카메라 전방으로 조금 밀어 등뒤/어깨뒤 오검출 완화
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|Aim", meta = (ClampMin = "0.0"))
    float AimTraceStartForwardOffset = 50.0f;

    // [추가] 총구 기준으로 이 Dot보다 큰 후보만 유효 aim hit로 인정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ranged|Aim", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float MinForwardDotForAimCandidate = 0.05f;

protected:
    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitGameplayEvent> FireEventTask;

    bool bHasCachedProfile = false;

    UPROPERTY()
    FRangedFireProfile CachedProfile;

protected:
    bool TryGetMuzzleFlashTransform(
        class ARangedWeaponBase* Weapon,
        FTransform& OutSpawnTransform
    ) const;

    bool SpawnMuzzleFlash(class ARangedWeaponBase* Weapon) const;

    virtual void FireCurrentProfile(class ARangedWeaponBase* Weapon);

    virtual void FireProjectileOnce(class ARangedWeaponBase* Weapon);

    virtual bool ApplyProjectileLaunchSettings(
        AActor* SpawnedProjectile,
        const FVector& ShotDirection
    ) const;

    virtual bool TryGetProjectileSpawnTransform(
        class ARangedWeaponBase* Weapon,
        FTransform& OutSpawnTransform,
        FVector& OutShotDirection
    ) const;

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

    bool TryGetCachedFinalDamage(
        ARangedWeaponBase* Weapon,
        float& OutFinalDamage
    ) const;

    bool GetViewPoint(FVector& OutLoc, FRotator& OutRot) const;

    void BuildTraceParams(FCollisionQueryParams& OutParams, bool bTraceComplex) const;

    bool TraceSingle(
        const FVector& Start,
        const FVector& End,
        const FRangedHitscanConfig& Hitscan,
        FHitResult& OutHit
    ) const;

    bool ResolveViewData(
        const FVector& FallbackLoc,
        const FRotator& FallbackRot,
        bool bUseControllerViewRotation,
        float SpreadHalfAngleDeg,
        FVector& OutViewLoc,
        FVector& OutViewDir
    ) const;

    // [추가] 카메라 ray 상의 후보들을 전부 수집
    bool TraceAimCandidatesFromView(
        const FVector& ViewLoc,
        const FVector& ViewDir,
        float TraceDistance,
        ECollisionChannel TraceChannel,
        bool bTraceComplex,
        float TraceRadius,
        TArray<FHitResult>& OutHits
    ) const;

    // [추가] 총구 기준 앞쪽 후보인지 검사
    bool IsForwardAimCandidate(
        const FVector& MuzzleLoc,
        const FVector& MuzzleForward,
        const FVector& CandidatePoint,
        float MinDotThreshold
    ) const;

    bool ComputeAimPointFromView(
        const FVector& ViewLoc,
        const FVector& ViewDir,
        float TraceDistance,
        ECollisionChannel TraceChannel,
        bool bTraceComplex,
        float TraceRadius,
        const FVector& MuzzleLoc,
        const FVector& MuzzleForward,
        FVector& OutAimPoint,
        FHitResult* OutViewHit = nullptr
    ) const;

    bool ComputeShotDirectionFromAimPoint(
        const FVector& MuzzleLoc,
        const FVector& AimPoint,
        const FVector& FallbackDir,
        FVector& OutShotDirection
    ) const;

    bool ComputeFinalHitscanHit(
        ARangedWeaponBase* Weapon,
        const FRangedHitscanConfig& Hitscan,
        FHitResult& OutFinalHit,
        FVector& OutAimPoint
    ) const;

    void FireHitscanOnce(ARangedWeaponBase* Weapon);

    bool ApplyRangedOnHitEffects(AActor* TargetActor) const;

    void HandleHitscanImpact(ARangedWeaponBase* Weapon, const FHitResult& FinalHit);

protected:
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