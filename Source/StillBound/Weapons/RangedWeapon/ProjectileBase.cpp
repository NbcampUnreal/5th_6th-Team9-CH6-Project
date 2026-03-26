#include "Weapons/RangedWeapon/ProjectileBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

#include "GameFramework/ProjectileMovementComponent.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

#include "TimerManager.h"

AProjectileBase::AProjectileBase()
{
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    SetRootComponent(CollisionComp);

    CollisionComp->InitSphereRadius(8.f);
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    CollisionComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
    CollisionComp->SetGenerateOverlapEvents(true);
    CollisionComp->SetNotifyRigidBodyCollision(false);
    CollisionComp->SetCanEverAffectNavigation(false);

    CollisionComp->SetSimulatePhysics(false);
    CollisionComp->SetEnableGravity(false);

    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMesh->SetupAttachment(CollisionComp);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ProjectileMesh->SetGenerateOverlapEvents(false);
    ProjectileMesh->SetCanEverAffectNavigation(false);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 0.f;
    ProjectileMovement->MaxSpeed = 0.f;
    ProjectileMovement->ProjectileGravityScale = 0.f;
    ProjectileMovement->bAutoActivate = false;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

    CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnProjectileBeginOverlap);
}

void AProjectileBase::BeginPlay()
{
    Super::BeginPlay();

    if (CollisionComp)
    {
        if (InitialCollisionDisableTime > 0.f)
        {
            DisableCollisionResponsesTemporarily();

            if (UWorld* World = GetWorld())
            {
                FTimerHandle TempHandle;
                World->GetTimerManager().SetTimer(
                    TempHandle,
                    this,
                    &ThisClass::RestoreCollisionResponsesAfterSpawnDelay,
                    InitialCollisionDisableTime,
                    false
                );
            }
        }
        else
        {
            RestoreCollisionResponsesAfterSpawnDelay();
        }
    }

    StartTrailFX();
}

void AProjectileBase::Destroyed()
{
    StopTrailFX(true);
    Super::Destroyed();
}

void AProjectileBase::InitProjectileData(
    AActor* InSourceInstigator,
    UAbilitySystemComponent* InSourceASC,
    TSubclassOf<UGameplayEffect> InBaseDamageEffectClass,
    float InFinalDamage,
    const TArray<FProjectileOnHitGameplayEffectSpec>& InOnHitTargetEffects,
    const FProjectileImpactFXPayload& InImpactFXPayload
)
{
    SourceInstigatorActor = InSourceInstigator;
    SourceASC = InSourceASC;
    BaseDamageEffectClass = InBaseDamageEffectClass;
    CachedFinalDamage = InFinalDamage;
    OnHitTargetEffects = InOnHitTargetEffects;
    CachedImpactFX = InImpactFXPayload;
    bHasImpactProcessed = false;
    bDeferredCollisionRestorePending = false;
}

void AProjectileBase::ConfigureImpulsePhysics(UPrimitiveComponent* InPhysicsComponent, bool bEnableGravity)
{
    if (!InPhysicsComponent)
    {
        return;
    }

    InPhysicsComponent->SetEnableGravity(bEnableGravity);

    if (InPhysicsComponent->IsSimulatingPhysics())
    {
        InPhysicsComponent->WakeAllRigidBodies();
    }
}

void AProjectileBase::DisableCollisionResponsesTemporarily()
{
    if (!CollisionComp || bHasImpactProcessed)
    {
        return;
    }

    bDeferredCollisionRestorePending = true;

    // QueryAndPhysics 유지 + 응답만 잠깐 Ignore
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void AProjectileBase::RestoreCollisionResponsesAfterSpawnDelay()
{
    if (!CollisionComp || bHasImpactProcessed)
    {
        return;
    }

    bDeferredCollisionRestorePending = false;

    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    CollisionComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
    CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
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

    if (!OtherActor || ShouldIgnoreActor(OtherActor))
    {
        return;
    }

    FHitResult HitResult = SweepResult;

    if (!bFromSweep)
    {
        HitResult = FHitResult(ForceInit);

        const FVector SelfLoc = GetActorLocation();
        const FVector OtherLoc = OtherComp ? OtherComp->GetComponentLocation() : OtherActor->GetActorLocation();

        HitResult.Location = SelfLoc;
        HitResult.ImpactPoint = OtherLoc;
        HitResult.TraceStart = SelfLoc;
        HitResult.TraceEnd = OtherLoc;

        const FVector Dir = (OtherLoc - SelfLoc).GetSafeNormal();
        HitResult.Normal = Dir.IsNearlyZero() ? FVector::UpVector : -Dir;
        HitResult.ImpactNormal = Dir.IsNearlyZero() ? FVector::UpVector : -Dir;
    }
    else
    {
        if (HitResult.ImpactPoint.IsNearlyZero())
        {
            HitResult.ImpactPoint = OtherComp ? OtherComp->GetComponentLocation() : OtherActor->GetActorLocation();
        }

        if (HitResult.Location.IsNearlyZero())
        {
            HitResult.Location = GetActorLocation();
        }

        if (HitResult.ImpactNormal.IsNearlyZero() && !HitResult.Normal.IsNearlyZero())
        {
            HitResult.ImpactNormal = HitResult.Normal;
        }
    }

    HandleImpact(HitResult, OtherActor);
}

void AProjectileBase::HandleImpact(const FHitResult& HitResult, AActor* ExplicitOtherActor)
{
    AActor* TargetActor = ExplicitOtherActor ? ExplicitOtherActor : HitResult.GetActor();

    if (TargetActor && ShouldIgnoreActor(TargetActor))
    {
        return;
    }

    if (bHasImpactProcessed)
    {
        return;
    }

    bHasImpactProcessed = true;
    bDeferredCollisionRestorePending = false;

    StopTrailFX(TrailFX.bDestroyOnImpact);

    SpawnImpactFXFromHitResult(HitResult);

    if (TargetActor)
    {
        ApplyDamageAndEffectsToTarget(TargetActor);
    }

    if (ProjectileMovement)
    {
        ProjectileMovement->StopMovementImmediately();
        ProjectileMovement->Deactivate();
    }

    if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(GetRootComponent()))
    {
        if (Prim->IsSimulatingPhysics())
        {
            Prim->SetPhysicsLinearVelocity(FVector::ZeroVector);
            Prim->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        }

        Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (bDestroyOnImpact)
    {
        Destroy();
    }
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

    if (SourceInstigatorActor && OtherActor == SourceInstigatorActor)
    {
        return true;
    }

    if (GetOwner() && OtherActor == GetOwner())
    {
        return true;
    }

    return false;
}

bool AProjectileBase::ApplyDamageAndEffectsToTarget(AActor* TargetActor) const
{
    if (!TargetActor || ShouldIgnoreActor(TargetActor))
    {
        return false;
    }

    bool bAnyApplied = false;

    if (CachedFinalDamage > 0.f)
    {
        bAnyApplied |= ApplyBaseDamageToTargetActor(TargetActor, CachedFinalDamage, 1.f, 1.f);
    }

    const FGameplayTag DamageTag = GetDataDamageTag();

    for (const FProjectileOnHitGameplayEffectSpec& Spec : OnHitTargetEffects)
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

    UAbilitySystemComponent* LocalSourceASC = SourceASC.Get();
    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

    if (!LocalSourceASC || !TargetASC)
    {
        return false;
    }

    AActor* SourceActor = SourceInstigatorActor.Get();
    if (!SourceActor)
    {
        SourceActor = GetOwner();
    }
    if (!SourceActor)
    {
        SourceActor = const_cast<AProjectileBase*>(this);
    }

    FGameplayEffectContextHandle Ctx = LocalSourceASC->MakeEffectContext();
    Ctx.AddInstigator(SourceActor, SourceActor);
    Ctx.AddSourceObject(const_cast<AProjectileBase*>(this));

    FGameplayEffectSpecHandle SpecHandle = LocalSourceASC->MakeOutgoingSpec(EffectClass, Level, Ctx);
    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
    {
        return false;
    }

    for (const auto& KVP : SetByCallerMagnitudes)
    {
        if (KVP.Key.IsValid())
        {
            SpecHandle.Data->SetSetByCallerMagnitude(KVP.Key, KVP.Value);
        }
    }

    LocalSourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
    return true;
}

bool AProjectileBase::ApplyBaseDamageToTargetActor(
    AActor* TargetActor,
    float DamageValue,
    float Level,
    float Chance
) const
{
    if (!TargetActor || DamageValue <= 0.f)
    {
        return false;
    }

    if (!BaseDamageEffectClass)
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

    return ApplyEffectToTargetActor(
        TargetActor,
        BaseDamageEffectClass,
        Level,
        Mags,
        Chance
    );
}

bool AProjectileBase::SpawnImpactFXFromHitResult(const FHitResult& HitResult) const
{
    if (!CachedImpactFX.IsConfigured())
    {
        return false;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FVector SpawnLocation = FVector::ZeroVector;
    FVector ImpactNormal = FVector::UpVector;

    if (!HitResult.ImpactPoint.IsNearlyZero())
    {
        SpawnLocation = HitResult.ImpactPoint;
    }
    else if (!HitResult.Location.IsNearlyZero())
    {
        SpawnLocation = HitResult.Location;
    }
    else if (HitResult.GetActor())
    {
        SpawnLocation = HitResult.GetActor()->GetActorLocation();
    }
    else
    {
        SpawnLocation = GetActorLocation();
    }

    if (!HitResult.ImpactNormal.IsNearlyZero())
    {
        ImpactNormal = HitResult.ImpactNormal;
    }
    else if (!HitResult.Normal.IsNearlyZero())
    {
        ImpactNormal = HitResult.Normal;
    }

    FRotator SpawnRotation = FRotator::ZeroRotator;
    if (CachedImpactFX.bUseImpactNormalRotation)
    {
        SpawnRotation = ImpactNormal.Rotation();
    }

    const FVector FinalLocation =
        SpawnLocation + SpawnRotation.RotateVector(CachedImpactFX.LocationOffset);

    const FRotator FinalRotation = SpawnRotation + CachedImpactFX.RotationOffset;

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        CachedImpactFX.NiagaraSystem,
        FinalLocation,
        FinalRotation,
        CachedImpactFX.Scale,
        true,
        true
    );

    return true;
}

void AProjectileBase::StartTrailFX()
{
    if (ActiveTrailComponent || !TrailFX.IsConfigured())
    {
        return;
    }

    USceneComponent* AttachParent = nullptr;

    if (TrailFX.bAttachToProjectile)
    {
        if (ProjectileMesh)
        {
            AttachParent = ProjectileMesh;
        }
        else
        {
            AttachParent = Cast<USceneComponent>(GetRootComponent());
        }
    }

    if (AttachParent)
    {
        ActiveTrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            TrailFX.NiagaraSystem,
            AttachParent,
            NAME_None,
            TrailFX.LocationOffset,
            TrailFX.RotationOffset,
            EAttachLocation::KeepRelativeOffset,
            false,
            true
        );

        if (ActiveTrailComponent)
        {
            ActiveTrailComponent->SetWorldScale3D(TrailFX.Scale);
        }

        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const FRotator BaseRotation = GetActorRotation();
    const FVector SpawnLocation =
        GetActorLocation() + BaseRotation.RotateVector(TrailFX.LocationOffset);
    const FRotator SpawnRotation = BaseRotation + TrailFX.RotationOffset;

    ActiveTrailComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        TrailFX.NiagaraSystem,
        SpawnLocation,
        SpawnRotation,
        TrailFX.Scale,
        false,
        true
    );
}

void AProjectileBase::StopTrailFX(bool bDestroyImmediately)
{
    if (!ActiveTrailComponent)
    {
        return;
    }

    if (bDestroyImmediately)
    {
        ActiveTrailComponent->DestroyComponent();
        ActiveTrailComponent = nullptr;
        return;
    }

    ActiveTrailComponent->Deactivate();
    ActiveTrailComponent = nullptr;
}

FGameplayTag AProjectileBase::GetDataDamageTag()
{
    static const FGameplayTag Tag =
        FGameplayTag::RequestGameplayTag(TEXT("Data.EnemyDamage"), false);

    ensureMsgf(
        Tag.IsValid(),
        TEXT("[ProjectileBase] GameplayTag 'Data.EnemyDamage' is not registered.")
    );

    return Tag;
}