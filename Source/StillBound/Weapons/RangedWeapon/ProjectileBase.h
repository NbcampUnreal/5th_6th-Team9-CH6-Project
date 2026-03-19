#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ProjectileBase.generated.h"

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

    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Projectile")
    void InitProjectileData(
        AActor* InSourceInstigator,
        AWeaponBase* InSourceWeapon,
        TSubclassOf<UGameplayEffect> InBaseDamageEffectClass,
        float InFinalDamage,
        const TArray<FProjectileOnHitGameplayEffectSpec>& InOnHitTargetEffects
    );

    UFUNCTION(BlueprintPure, Category = "Weapon|Projectile")
    UStaticMeshComponent* GetProjectileMesh() const { return ProjectileMesh; }

    UFUNCTION(BlueprintPure, Category = "Weapon|Projectile")
    UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

    // 수정: 임펄스 모드용 물리 중력 배율 설정
    void ConfigureImpulsePhysics(UPrimitiveComponent* InPhysicsComponent, float InGravityScale);

protected:
    virtual void BeginPlay() override;

protected:
    // 수정:
    // 메쉬 자체를 충돌 주체로 사용
    // 충돌 채널/프리셋은 BP에서 설정
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

    // 수정: 임펄스 모드에서 GravityScale > 1 지원용
    TWeakObjectPtr<UPrimitiveComponent> ImpulsePhysicsComponent;
    bool bUseCustomImpulseGravity = false;
    float ImpulseGravityScale = 0.f;

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

    void SetupIgnoredActors();
};