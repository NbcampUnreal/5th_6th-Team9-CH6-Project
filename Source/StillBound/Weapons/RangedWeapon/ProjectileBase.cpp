#include "Weapons/RangedWeapon/ProjectileBase.h"

#include "Weapons/WeaponBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"

#include "GameFramework/ProjectileMovementComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

AProjectileBase::AProjectileBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // 수정:
    // 메쉬 자체를 충돌 주체로 사용
    // 충돌 채널/프리셋은 BP에서 설정
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    SetRootComponent(ProjectileMesh);

    // 수정: 생성자 최소화
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ProjectileMesh->SetNotifyRigidBodyCollision(true);
    ProjectileMesh->SetGenerateOverlapEvents(true);
    ProjectileMesh->SetCanEverAffectNavigation(false);

    // 수정:
    // 고속 임펄스 관통 완화용 CCD
    // 실제 충돌 형태는 메쉬 에셋의 simple collision 사용
    ProjectileMesh->BodyInstance.SetUseCCD(true);

    // 수정:
    // 기본은 물리 off
    // 임펄스 모드에서만 Ability 쪽에서 켠다
    ProjectileMesh->SetSimulatePhysics(false);
    ProjectileMesh->SetEnableGravity(false);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = ProjectileMesh;

    // 수정: 자동 발사 방지. 발사 시점에 GA가 값 넣는다.
    ProjectileMovement->InitialSpeed = 0.f;
    ProjectileMovement->MaxSpeed = 0.f;
    ProjectileMovement->ProjectileGravityScale = 0.f;
    ProjectileMovement->Velocity = FVector::ZeroVector;
    ProjectileMovement->bAutoActivate = false;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

    ProjectileMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnProjectileHit);
    ProjectileMesh->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnProjectileBeginOverlap);
}

void AProjectileBase::BeginPlay()
{
    Super::BeginPlay();


    SetActorTickEnabled(false);
}

void AProjectileBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bUseCustomImpulseGravity)
    {
        return;
    }

    UPrimitiveComponent* Prim = ImpulsePhysicsComponent.Get();
    UWorld* World = GetWorld();
    if (!Prim || !World || !Prim->IsSimulatingPhysics())
    {
        bUseCustomImpulseGravity = false;
        ImpulsePhysicsComponent = nullptr;
        SetActorTickEnabled(false);
        return;
    }

    // GravityScale <= 0 은 무중력 처리
    if (ImpulseGravityScale <= 0.f)
    {
        return;
    }

    // 1.0 은 엔진 기본 중력만 사용
    if (FMath::IsNearlyEqual(ImpulseGravityScale, 1.f))
    {
        return;
    }

    // 엔진 기본 중력 1배 외 추가 배율만 보정
    const float ExtraGravityScale = ImpulseGravityScale - 1.f;
    const float GravityZ = World->GetGravityZ(); // 보통 음수
    const FVector ExtraForce = FVector(0.f, 0.f, Prim->GetMass() * GravityZ * ExtraGravityScale);

    Prim->AddForce(ExtraForce, NAME_None, true);
}

void AProjectileBase::InitProjectileData(
    AActor* InSourceInstigator,
    AWeaponBase* InSourceWeapon,
    TSubclassOf<UGameplayEffect> InBaseDamageEffectClass,
    float InFinalDamage,
    const TArray<FProjectileOnHitGameplayEffectSpec>& InOnHitTargetEffects
)
{
    SourceInstigatorActor = InSourceInstigator;
    SourceWeapon = InSourceWeapon;
    BaseDamageEffectClass = InBaseDamageEffectClass;
    CachedFinalDamage = InFinalDamage;
    OnHitTargetEffects = InOnHitTargetEffects;
    bHasImpactProcessed = false;
    SetupIgnoredActors();
}

void AProjectileBase::ConfigureImpulsePhysics(UPrimitiveComponent* InPhysicsComponent, float InGravityScale)
{
    ImpulsePhysicsComponent = InPhysicsComponent;
    ImpulseGravityScale = FMath::Max(0.f, InGravityScale);

    if (!InPhysicsComponent)
    {
        bUseCustomImpulseGravity = false;
        SetActorTickEnabled(false);
        return;
    }

    InPhysicsComponent->SetEnableGravity(ImpulseGravityScale > 0.f);

    bUseCustomImpulseGravity =
        (ImpulseGravityScale > 0.f) &&
        !FMath::IsNearlyEqual(ImpulseGravityScale, 1.f);

    SetActorTickEnabled(bUseCustomImpulseGravity);
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

    FHitResult HitResult = SweepResult;

    if (!bFromSweep)
    {
        // 수정:
        // FHitResult에 Actor 직접 대입하지 않음
        // 대상 액터는 HandleImpact의 ExplicitOtherActor로 전달
        HitResult.Location = GetActorLocation();
        HitResult.ImpactPoint = GetActorLocation();

        if (OtherActor)
        {
            HitResult.TraceEnd = OtherActor->GetActorLocation();
            HitResult.Normal = (GetActorLocation() - OtherActor->GetActorLocation()).GetSafeNormal();
            HitResult.ImpactNormal = HitResult.Normal;
        }
        else
        {
            HitResult.TraceEnd = GetActorLocation();
            HitResult.Normal = FVector::UpVector;
            HitResult.ImpactNormal = FVector::UpVector;
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

    SpawnWeaponHitImpactFXFromHitResult(HitResult);

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

    if (SourceInstigatorActor.IsValid() && OtherActor == SourceInstigatorActor.Get())
    {
        return true;
    }

    if (SourceWeapon.IsValid() && OtherActor == SourceWeapon.Get())
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

    AActor* SourceActor = SourceInstigatorActor.Get();
    UAbilitySystemComponent* SourceASC =
        SourceActor ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor) : nullptr;

    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

    if (!SourceASC || !TargetASC)
    {
        return false;
    }

    AWeaponBase* Weapon = SourceWeapon.Get();

    FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
    if (SourceActor)
    {
        Ctx.AddInstigator(SourceActor, Weapon ? Cast<AActor>(Weapon) : SourceActor);
    }

    if (Weapon)
    {
        Ctx.AddSourceObject(Weapon);
    }

    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, Level, Ctx);
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
    if (FX.bUseImpactNormalRotation)
    {
        SpawnRotation = ImpactNormal.Rotation();
    }

    const FVector FinalLocation =
        SpawnLocation + SpawnRotation.RotateVector(FX.LocationOffset);

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

    ensureMsgf(
        Tag.IsValid(),
        TEXT("[ProjectileBase] GameplayTag 'Data.EnemyDamage' is not registered.")
    );

    return Tag;
}

void AProjectileBase::SetupIgnoredActors()
{
    if (!ProjectileMesh)
    {
        return;
    }

    TArray<AActor*> ActorsToIgnore;

    if (AActor* OwnerActor = GetOwner())
    {
        ActorsToIgnore.AddUnique(OwnerActor);
    }

    if (SourceInstigatorActor.IsValid())
    {
        ActorsToIgnore.AddUnique(SourceInstigatorActor.Get());
    }

    if (SourceWeapon.IsValid())
    {
        ActorsToIgnore.AddUnique(SourceWeapon.Get());
    }

    for (AActor* ActorToIgnore : ActorsToIgnore)
    {
        if (!ActorToIgnore)
        {
            continue;
        }

        // 프로젝타일이 이동할 때 이 액터를 무시
        ProjectileMesh->IgnoreActorWhenMoving(ActorToIgnore, true);

       

        // 상대쪽 컴포넌트도 가능하면 이 프로젝타일을 무시
        TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(ActorToIgnore);
        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
        {
            if (PrimComp)
            {
                PrimComp->IgnoreActorWhenMoving(this, true);
            }
        }
    }
}