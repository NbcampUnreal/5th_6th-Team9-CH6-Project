// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ProjectileBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UPrimitiveComponent;
class UGameplayEffect;
class AWeaponBase;

USTRUCT(BlueprintType)
struct FProjectileOnHitGameplayEffectSpec
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Projectile|OnHit")
    TSubclassOf<UGameplayEffect> Effect = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Projectile|OnHit")
    float Level = 1.f;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Projectile|OnHit")
    TMap<FGameplayTag, float> SetByCallerMagnitudes;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Projectile|OnHit", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Chance = 1.0f;
};

UCLASS(Abstract, Blueprintable)
class STILLBOUND_API AProjectileBase : public AActor
{
    GENERATED_BODY()

public:
    AProjectileBase();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Projectile")
    void InitProjectileData(
        AActor* InSourceInstigator,
        AWeaponBase* InSourceWeapon,
        TSubclassOf<UGameplayEffect> InBaseDamageEffectClass,
        float InFinalDamage,
        const TArray<FProjectileOnHitGameplayEffectSpec>& InOnHitTargetEffects
    );

    UFUNCTION(BlueprintPure, Category = "Weapon|Projectile")
    USphereComponent* GetCollisionComp() const { return CollisionComp; }

    UFUNCTION(BlueprintPure, Category = "Weapon|Projectile")
    UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

protected:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Projectile")
    TObjectPtr<USphereComponent> CollisionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Projectile")
    TObjectPtr<UStaticMeshComponent> ProjectileMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Projectile")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Projectile")
    bool bDestroyOnImpact = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Projectile")
    bool bUseOverlapAsFallback = true;

protected:
    UPROPERTY()
    TWeakObjectPtr<AActor> SourceInstigatorActor;

    UPROPERTY()
    TWeakObjectPtr<AWeaponBase> SourceWeapon;

    UPROPERTY()
    TSubclassOf<UGameplayEffect> BaseDamageEffectClass;

    UPROPERTY()
    float CachedFinalDamage = 0.f;

    UPROPERTY()
    TArray<FProjectileOnHitGameplayEffectSpec> OnHitTargetEffects;

    UPROPERTY()
    bool bHasImpactProcessed = false;

protected:
    UFUNCTION()
    void OnProjectileHit(
        UPrimitiveComponent* HitComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector NormalImpulse,
        const FHitResult& Hit
    );

    UFUNCTION()
    void OnProjectileBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

protected:
    void HandleImpact(const FHitResult& HitResult, AActor* ExplicitOtherActor = nullptr);

    bool ShouldIgnoreActor(AActor* OtherActor) const;

    bool ApplyDamageAndEffectsToTarget(AActor* TargetActor) const;

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

    bool SpawnWeaponHitImpactFXFromHitResult(const FHitResult& HitResult) const;

    static FGameplayTag GetDataDamageTag();
};