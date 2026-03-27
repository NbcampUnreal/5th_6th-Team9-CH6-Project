#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "Engine/EngineTypes.h"
#include "WeaponGameplayAbility.generated.h"

class AWeaponBase;
class UGameplayEffect;
class UAbilitySystemComponent;

UCLASS(Abstract)
class STILLBOUND_API UWeaponGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UWeaponGameplayAbility();

protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

    virtual bool CheckCost(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        FGameplayTagContainer* OptionalRelevantTags = nullptr
    ) const override;

    virtual void ApplyCost(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo
    ) const override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled
    ) override;

    void AddFaceAimStateTag();
    void RemoveFaceAimStateTag();

protected:
    UFUNCTION(BlueprintCallable, Category = "Weapon|GA")
    AWeaponBase* GetWeaponFromSourceObject() const;

    AWeaponBase* GetWeaponFromSourceObjectByHandle(const FGameplayAbilitySpecHandle Handle) const;

    template<typename T>
    T* GetWeaponFromSourceObject() const
    {
        return Cast<T>(GetWeaponFromSourceObject());
    }

    bool ApplyEffectToTargetActor(
        AActor* TargetActor,
        TSubclassOf<UGameplayEffect> EffectClass,
        float Level,
        const TMap<FGameplayTag, float>& SetByCallerMagnitudes,
        float Chance = 1.0f
    ) const;

    bool ApplyBaseDamageToTargetActor(
        AActor* TargetActor,
        float DamageValue,
        float Level = 1.0f,
        float Chance = 1.0f
    ) const;

    bool ApplyEffectToSelf(
        TSubclassOf<UGameplayEffect> EffectClass,
        float Level,
        const TMap<FGameplayTag, float>& SetByCallerMagnitudes,
        float Chance = 1.0f
    ) const;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Damage")
    TSubclassOf<UGameplayEffect> BaseDamageEffectClass;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Debug")
    bool bDebugGE = true;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Debug")
    bool bDebugHitFX = true;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|State")
    bool bUseFaceAimStateTag = true;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|State")
    FGameplayTag FaceAimStateTag;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Debug")
    bool bDebugStateTag = true;

    UPROPERTY(Transient)
    bool bAddedFaceAimStateTagThisActivation = false;

    UFUNCTION(BlueprintPure, Category = "Weapon|Damage")
    float GetDamageFromWeaponOrFallback(float FallbackDamage = 0.f) const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Damage")
    bool ApplyWeaponDamageToTargetActor(
        AActor* TargetActor,
        float DamageMultiplier = 1.f,
        float Level = 1.f,
        float Chance = 1.f,
        float FallbackDamage = 0.f
    ) const;

    bool SpawnWeaponHitImpactFXFromHitResult(const FHitResult& HitResult) const;

    bool SpawnWeaponHitImpactFXAtLocation(
        const FVector& SpawnLocation,
        const FVector& ImpactNormal = FVector::UpVector
    ) const;

protected:
    const FGameplayAbilitySpec* FindCurrentAbilitySpec() const;
    const FGameplayAbilitySpec* FindAbilitySpecByHandle(const FGameplayAbilitySpecHandle Handle) const;

    UFUNCTION(BlueprintPure, Category = "Weapon|GA")
    FGameplayTag GetInputTagFromCurrentSpec() const;

    bool TryGetInputTagFromCurrentSpec(FGameplayTag& OutInputTag) const;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Stamina")
    bool bUseAttackStaminaCost = false;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Stamina")
    bool bApplyAttackStaminaCostOnActivate = false;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Stamina")
    TSubclassOf<UGameplayEffect> AttackStaminaCostEffectClass;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Stamina")
    FGameplayTag StaminaCostSetByCallerTag;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Stamina")
    float AttackStaminaCostMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Stamina")
    float AttackStaminaCostFlatDelta = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Debug")
    bool bDebugStaminaCost = true;

protected:
    UFUNCTION(BlueprintPure, Category = "Weapon|Stamina")
    float ComputeAttackStaminaCostFromDamage(float DamageValue) const;

    bool TryComputeAttackStaminaCostFromWeapon(
        const FGameplayAbilitySpecHandle Handle,
        float& OutFinalCost,
        float DamageMultiplier = 1.f,
        float FallbackDamage = 0.f
    ) const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Stamina")
    bool ApplyAttackStaminaCostValueToSelf(
        float FinalCost,
        float Level = 1.f,
        float Chance = 1.f
    ) const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Stamina")
    bool ApplyAttackStaminaCostToSelf(
        float DamageValue,
        float Level = 1.f,
        float Chance = 1.f
    ) const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Stamina")
    bool ApplyAttackStaminaCostFromWeapon(
        float DamageMultiplier = 1.f,
        float Level = 1.f,
        float Chance = 1.f,
        float FallbackDamage = 0.f
    ) const;

public:
    static FGameplayTag GetDataDamageTag();
    static FGameplayTag GetDataStaminaCostTag();
};