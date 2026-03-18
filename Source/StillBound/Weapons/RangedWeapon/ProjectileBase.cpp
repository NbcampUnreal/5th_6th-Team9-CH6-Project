// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/RangedWeapon/ProjectileBase.h"

#include "Weapons/WeaponBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"

#include "GameFramework/ProjectileMovementComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

AProjectileBase::AProjectileBase()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    SetRootComponent(CollisionComp);

    CollisionComp->InitSphereRadius(8.f);
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    CollisionComp->SetGenerateOverlapEvents(true);
    CollisionComp->SetNotifyRigidBodyCollision(true);
    CollisionComp->SetCanEverAffectNavigation(false);

    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMesh->SetupAttachment(CollisionComp);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ProjectileMesh->SetGenerateOverlapEvents(false);
    ProjectileMesh->SetCanEverAffectNavigation(false);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 3000.f;
    ProjectileMovement->MaxSpeed = 3000.f;
    ProjectileMovement->ProjectileGravityScale = 0.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

    CollisionComp->OnComponentHit.AddDynamic(this, &ThisClass::OnProjectileHit);
    CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnProjectileBeginOverlap);
}

void AProjectileBase::BeginPlay()
{
    Super::BeginPlay();
}

void AProjectileBase::InitProjectileData(
    AActor* InSourceInstigator,
    AWeaponBase* InSourceWeapon,
    TSubclassOf<UGameplayEffect> InBaseDamageEffectClass,
    float InDamageMultiplier,
    const TArray<FRangedOnHitGameplayEffectSpec>& InOnHitTargetEffects
)
{
    SourceInstigatorActor = InSourceInstigator;
    SourceWeapon = InSourceWeapon;
    BaseDamageEffectClass = InBaseDamageEffectClass;
    DamageMultiplier = InDamageMultiplier;
    OnHitTargetEffects = InOnHitTargetEffects;

    if (CollisionComp)
    {
        if (InSourceInstigator)
        {
            CollisionComp->IgnoreActorWhenMoving(InSourceInstigator, true);
        }

        if (InSourceWeapon)
        {
            CollisionComp->IgnoreActorWhenMoving(InSourceWeapon, true);
        }

        if (AActor* OwnerActor = GetOwner())
        {
            CollisionComp->IgnoreActorWhenMoving(OwnerActor, true);
        }
    }
}

void AProjectileBase::OnProjectileHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector NormalImpulse,
    const FHitResult& Hit
)
{
    HandleImpact(Hit, OtherActor);
}

void AProjectileBase::OnProjectileBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    if (!bUseOverlapAsFallback)
    {
        return;
    }

    HandleImpact(SweepResult, OtherActor);
}

bool AProjectileBase::ShouldIgnoreActor(AActor* OtherActor) const
{
    if (!OtherActor)
    {
        return false;
    }

    if (OtherActor == this)
    {
        return true;
    }

    if (OtherActor == GetOwner())
    {
        return true;
    }

    if (OtherActor == GetInstigator())
    {
        return true;
    }

    if (OtherActor == SourceInstigatorActor.Get())
    {
        return true;
    }

    if (OtherActor == SourceWeapon.Get())
    {
        return true;
    }

    return false;
}

void AProjectileBase::HandleImpact(const FHitResult& HitResult, AActor* ExplicitOtherActor)
{
    if (bHasImpactProcessed)
    {
        return;
    }

    AActor* HitActor = HitResult.GetActor();
    if (!HitActor)
    {
        HitActor = ExplicitOtherActor;
    }

    if (ShouldIgnoreActor(HitActor))
    {
        return;
    }

    bHasImpactProcessed = true;

    // 월드/벽 포함해서 FX는 먼저
    SpawnWeaponHitImpactFXFromHitResult(HitResult);

    if (HitActor)
    {
        ApplyDamageAndEffectsToTarget(HitActor);
    }

    if (bDestroyOnImpact)
    {
        Destroy();
    }
}

bool AProjectileBase::ApplyDamageAndEffectsToTarget(AActor* TargetActor) const
{
    if (!TargetActor)
    {
        return false;
    }

    bool bAnyApplied = false;

    const AWeaponBase* Weapon = SourceWeapon.Get();
    const float BaseDamage = Weapon ? Weapon->GetWeaponDamage() : 0.f;
    const float FinalDamage = BaseDamage * FMath::Max(0.f, DamageMultiplier);

    if (FinalDamage > 0.f)
    {
        bAnyApplied |= ApplyBaseDamageToTargetActor(TargetActor, FinalDamage, 1.0f, 1.0f);
    }

    const FGameplayTag DamageTag = GetDataDamageTag();

    for (const FRangedOnHitGameplayEffectSpec& Spec : OnHitTargetEffects)
    {
        if (!Spec.Effect)
        {
            continue;
        }

        TMap<FGameplayTag, float> Mags = Spec.SetByCallerMagnitudes;
        if (DamageTag.IsValid())
        {
            Mags.Remove(DamageTag);
        }

        bAnyApplied |= ApplyEffectToTargetActor(
            TargetActor,
            Spec.Effect,
            Spec.Level,
            Mags,
            Spec.Chance
        );
    }

    return bAnyApplied;
}

bool AProjectileBase::ApplyEffectToTargetActor(
    AActor* TargetActor,
    TSubclassOf<UGameplayEffect> EffectClass,
    float Level,
    const TMap<FGameplayTag, float>& SetByCallerMagnitudes,
    float Chance
) const
{
    if (!TargetActor || !EffectClass)
    {
        return false;
    }

    if (Chance < 1.0f)
    {
        const float Roll = FMath::FRand();
        if (Roll > Chance)
        {
            return false;
        }
    }

    AActor* InstigatorActor = SourceInstigatorActor.Get();
    if (!InstigatorActor)
    {
        InstigatorActor = GetOwner();
    }

    UAbilitySystemComponent* SourceASC =
        InstigatorActor ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InstigatorActor) : nullptr;

    if (!SourceASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Projectile] SourceASC is null. Projectile=%s"), *GetNameSafe(this));
        return false;
    }

    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

    if (!TargetASC)
    {
        return false;
    }

    AWeaponBase* Weapon = SourceWeapon.Get();

    FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
    if (InstigatorActor)
    {
        Ctx.AddInstigator(InstigatorActor, Weapon ? Cast<AActor>(Weapon) : InstigatorActor);
    }
    if (Weapon)
    {
        Ctx.AddSourceObject(Weapon);
    }

    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, Level, Ctx);
    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Projectile] MakeOutgoingSpec failed. Effect=%s Projectile=%s"),
            *GetNameSafe(EffectClass), *GetNameSafe(this));
        return false;
    }

    for (const auto& KVP : SetByCallerMagnitudes)
    {
        if (KVP.Key.IsValid())
        {
            SpecHandle.Data->SetSetByCallerMagnitude(KVP.Key, KVP.Value);
        }
    }

    SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
    return true;
}

bool AProjectileBase::ApplyBaseDamageToTargetActor(
    AActor* TargetActor,
    float DamageValue,
    float Level,
    float Chance
) const
{
    if (!TargetActor || DamageValue <= 0.f || !BaseDamageEffectClass)
    {
        return false;
    }

    const FGameplayTag DamageTag = GetDataDamageTag();
    if (!DamageTag.IsValid())
    {
        return false;
    }

    TMap<FGameplayTag, float> Mags;
    Mags.Add(DamageTag, DamageValue);

    UE_LOG(LogTemp, Log,
        TEXT("[Projectile] ApplyDamage Final=%.2f Target=%s Weapon=%s Projectile=%s"),
        DamageValue,
        *GetNameSafe(TargetActor),
        *GetNameSafe(SourceWeapon.Get()),
        *GetNameSafe(this));

    return ApplyEffectToTargetActor(
        TargetActor,
        BaseDamageEffectClass,
        Level,
        Mags,
        Chance
    );
}

bool AProjectileBase::SpawnWeaponHitImpactFXFromHitResult(const FHitResult& HitResult) const
{
    const AWeaponBase* Weapon = SourceWeapon.Get();
    if (!Weapon)
    {
        return false;
    }

    const FWeaponHitImpactFX& FX = Weapon->GetHitImpactFX();
    if (!FX.NiagaraSystem)
    {
        return false;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FVector SpawnLocation = HitResult.ImpactPoint;
    if (SpawnLocation.IsNearlyZero())
    {
        SpawnLocation = HitResult.Location;
    }
    if (SpawnLocation.IsNearlyZero())
    {
        SpawnLocation = GetActorLocation();
    }

    FVector ImpactNormal = HitResult.ImpactNormal;
    if (ImpactNormal.IsNearlyZero())
    {
        ImpactNormal = HitResult.Normal;
    }
    if (ImpactNormal.IsNearlyZero())
    {
        ImpactNormal = FVector::UpVector;
    }

    FRotator SpawnRotation = FRotator::ZeroRotator;
    if (FX.bUseImpactNormalRotation)
    {
        SpawnRotation = ImpactNormal.Rotation();
    }

    const FVector FinalLocation = SpawnLocation + SpawnRotation.RotateVector(FX.LocationOffset);
    const FRotator FinalRotation = SpawnRotation + FX.RotationOffset;

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        FX.NiagaraSystem,
        FinalLocation,
        FinalRotation,
        FX.Scale,
        true,
        true
    );

    return true;
}

FGameplayTag AProjectileBase::GetDataDamageTag()
{
    static const FGameplayTag Tag =
        FGameplayTag::RequestGameplayTag(TEXT("Data.EnemyDamage"), false);
    return Tag;
}