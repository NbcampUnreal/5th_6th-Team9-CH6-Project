#pragma once

#include "CoreMinimal.h"
#include "Weapons/GameAbility/WeaponGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"     // FGameplayEventData
#include "Engine/EngineTypes.h"      // FHitResult
#include "Weapons/MeleeWeaponBase/MeleeWeaponBase.h" // FWeaponAttackProfile
#include "WeaponMeleeAttackAbilityBase.generated.h"

class AMeleeWeaponBase;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UPrimitiveComponent;

UCLASS(Abstract)
class STILLBOUND_API UWeaponMeleeAttackAbilityBase : public UWeaponGameplayAbility
{
    GENERATED_BODY()

public:
    UWeaponMeleeAttackAbilityBase();

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee|HitWindow")
    FGameplayTag HitWindowOnEventTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee|HitWindow")
    FGameplayTag HitWindowOffEventTag;

    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitGameplayEvent> HitWindowOnTask;

    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitGameplayEvent> HitWindowOffTask;

    bool bHasCachedProfile = false;

    UPROPERTY()
    FWeaponAttackProfile CachedProfile;

    // 같은 액터 중복 타격 방지용
    TSet<TWeakObjectPtr<AActor>> HitActors;

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

    bool CacheProfileFromWeapon(AMeleeWeaponBase* Weapon);

    void BindHitBoxOverlap(AMeleeWeaponBase* Weapon);
    void UnbindHitBoxOverlap(AMeleeWeaponBase* Weapon);

    UFUNCTION()
    void OnHitWindowOnEventReceived(FGameplayEventData Payload);

    UFUNCTION()
    void OnHitWindowOffEventReceived(FGameplayEventData Payload);

    UFUNCTION()
    void OnHitBoxBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    bool ApplyOnHitEffects(AActor* TargetActor);

    UFUNCTION()
    void OnMontageCompleted();

    UFUNCTION()
    void OnMontageCancelled();

    UFUNCTION()
    void OnMontageInterrupted();

    UFUNCTION()
    void OnMontageBlendOut();
};