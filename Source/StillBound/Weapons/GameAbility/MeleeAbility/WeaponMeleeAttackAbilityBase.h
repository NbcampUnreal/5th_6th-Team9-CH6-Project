#pragma once

#include "CoreMinimal.h"
#include "Weapons/GameAbility/WeaponGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "Weapons/MeleeWeaponBase/MeleeWeaponBase.h"
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
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee")
    FGameplayTag AttackTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee|HitWindow")
    FGameplayTag HitWindowOnEventTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee|HitWindow")
    FGameplayTag HitWindowOffEventTag;

    /** (테스트/임시) 기본 데미지. 0이면 사용 안 함 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee|Damage")
    float BaseDamage = 0.f;

    // =========================
    // Debug (Begin/End HitBox)
    // =========================
    /** 히트 윈도우 ON/OFF 시점에만 “히트박스 모양”을 1회 그린다 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee|Debug")
    bool bDebugHitBoxBeginEnd ;

    /** Begin/End 박스가 남아있을 시간(초). 0이면 1프레임 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee|Debug", meta = (ClampMin = "0.0"))
    float DebugBeginEndLifeTime ;

    /** 박스 라인 두께 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Melee|Debug", meta = (ClampMin = "0.1"))
    float DebugBoxThickness = 2.0f;

    /** Begin/End에 1회 히트박스 모양 출력 */
    void DrawHitBoxOnce(AMeleeWeaponBase* Weapon, const FColor& Color, float LifeTime) const;

    // --- runtime ---
    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitGameplayEvent> HitWindowOnTask;

    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitGameplayEvent> HitWindowOffTask;

    UPROPERTY()
    TSet<TWeakObjectPtr<AActor>> HitActors;

    bool bHasCachedProfile = false;
    FWeaponAttackProfile CachedProfile;

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

    void EnableHitWindow(AMeleeWeaponBase* Weapon);
    void DisableHitWindow(AMeleeWeaponBase* Weapon);

    bool ApplyOnHitEffects(AActor* TargetActor);

    // --- AbilityTask callbacks ---
    UFUNCTION()
    void OnHitWindowOnEventReceived(FGameplayEventData Payload);

    UFUNCTION()
    void OnHitWindowOffEventReceived(FGameplayEventData Payload);

    // --- Overlap callback ---
    UFUNCTION()
    void OnHitBoxBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION()
    void OnMontageCompleted();

    UFUNCTION()
    void OnMontageCancelled();

    UFUNCTION()
    void OnMontageInterrupted();

    UFUNCTION()
    void OnMontageBlendOut();
};
