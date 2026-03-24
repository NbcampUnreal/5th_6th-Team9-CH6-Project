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

    bool ComputeAimPointFromView(
        const FVector& ViewLoc,
        const FVector& ViewDir,
        float TraceDistance,
        ECollisionChannel TraceChannel,
        bool bTraceComplex,
        float TraceRadius,
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