// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/GameAbility/RangedAbility/WeaponRangedAttackAbilityBase.h"

#include "Weapons/RangedWeapon/RangedWeaponBase.h"
#include "Weapons/RangedWeapon/ProjectileBase.h" // 추가: Projectile 전용 초기화/물리 설정 호출용
#include "Weapons/WeaponBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

UWeaponRangedAttackAbilityBase::UWeaponRangedAttackAbilityBase()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    FireEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Ranged.Fire"), false);

    bFireImmediatelyIfNoMontageOrEvent = true;

    bDebugTrace = true;
    DebugLifeTime = 1.0f;
    DebugLineThickness = 1.5f;
}

void UWeaponRangedAttackAbilityBase::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData
)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    ARangedWeaponBase* Weapon = GetWeaponFromSourceObject<ARangedWeaponBase>();
    if (!Weapon)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }

    if (!CacheFireProfileFromWeapon(Weapon))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }

    if (FireEventTag.IsValid())
    {
        FireEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
            this, FireEventTag, nullptr, false, true);

        if (FireEventTask)
        {
            FireEventTask->EventReceived.AddDynamic(this, &ThisClass::OnFireEventReceived);
            FireEventTask->ReadyForActivation();
        }
    }

    if (CachedProfile.Montage)
    {
        MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, CachedProfile.Montage, CachedProfile.MontagePlayRate, NAME_None, true);

        if (!MontageTask)
        {
            EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
            return;
        }

        MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
        MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
        MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
        MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageBlendOut);
        MontageTask->ReadyForActivation();

        if (bFireImmediatelyIfNoMontageOrEvent && !FireEventTag.IsValid())
        {
            FireCurrentProfile(Weapon);
        }

        return;
    }

    if (bFireImmediatelyIfNoMontageOrEvent)
    {
        FireCurrentProfile(Weapon);
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UWeaponRangedAttackAbilityBase::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled
)
{
    bHasCachedProfile = false;
    MontageTask = nullptr;
    FireEventTask = nullptr;

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UWeaponRangedAttackAbilityBase::CacheFireProfileFromWeapon(ARangedWeaponBase* Weapon)
{
    bHasCachedProfile = false;
    if (!Weapon)
    {
        return false;
    }

    const FGameplayTag InputTag = GetInputTagFromCurrentSpec();
    if (!InputTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[RangedGA] Missing InputTag in AbilitySpec. Weapon=%s Ability=%s"),
            *GetNameSafe(Weapon), *GetNameSafe(this));
        return false;
    }

    FRangedFireProfile Profile;
    if (!Weapon->GetFireProfile(InputTag, Profile))
    {
        UE_LOG(LogTemp, Warning, TEXT("[RangedGA] GetFireProfile failed InputTag=%s Weapon=%s"),
            *InputTag.ToString(), *GetNameSafe(Weapon));
        return false;
    }

    CachedProfile = Profile;
    bHasCachedProfile = true;
    return true;
}

bool UWeaponRangedAttackAbilityBase::TryGetCachedFinalDamage(
    ARangedWeaponBase* Weapon,
    float& OutFinalDamage
) const
{
    OutFinalDamage = 0.f;

    if (!Weapon || !bHasCachedProfile)
    {
        return false;
    }

    OutFinalDamage = Weapon->CalculateFinalDamageFromProfile(CachedProfile);
    return true;
}

bool UWeaponRangedAttackAbilityBase::GetViewPoint(FVector& OutLoc, FRotator& OutRot) const
{
    const FGameplayAbilityActorInfo* Info = CurrentActorInfo;
    if (!Info)
    {
        return false;
    }

    APawn* Pawn = Cast<APawn>(Info->AvatarActor.Get());
    AController* Controller = Info->PlayerController.Get();

    if (!Controller && Pawn)
    {
        Controller = Pawn->GetController();
    }

    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        PC->GetPlayerViewPoint(OutLoc, OutRot);
        return true;
    }

    if (Pawn && Controller)
    {
        OutLoc = Pawn->GetPawnViewLocation();
        OutRot = Controller->GetControlRotation();
        return true;
    }

    if (AActor* Avatar = Info->AvatarActor.Get())
    {
        OutLoc = Avatar->GetActorLocation();
        OutRot = Avatar->GetActorRotation();
        return true;
    }

    return false;
}

void UWeaponRangedAttackAbilityBase::BuildTraceParams(FCollisionQueryParams& OutParams, bool bTraceComplex) const
{
    OutParams.bTraceComplex = bTraceComplex;

    if (AActor* Avatar = GetAvatarActorFromActorInfo())
    {
        OutParams.AddIgnoredActor(Avatar);
    }

    if (AWeaponBase* Weapon = GetWeaponFromSourceObject<AWeaponBase>())
    {
        OutParams.AddIgnoredActor(Weapon);
    }
}

bool UWeaponRangedAttackAbilityBase::TraceSingle(
    const FVector& Start,
    const FVector& End,
    const FRangedHitscanConfig& Hitscan,
    FHitResult& OutHit
) const
{
    AActor* Avatar = GetAvatarActorFromActorInfo();
    UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
    if (!World)
    {
        return false;
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponRangedTrace), Hitscan.bTraceComplex);
    BuildTraceParams(Params, Hitscan.bTraceComplex);

    const bool bUseSphere = (Hitscan.Radius > KINDA_SMALL_NUMBER);
    if (!bUseSphere)
    {
        return World->LineTraceSingleByChannel(OutHit, Start, End, Hitscan.TraceChannel, Params);
    }

    const FCollisionShape Shape = FCollisionShape::MakeSphere(Hitscan.Radius);
    return World->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, Hitscan.TraceChannel, Shape, Params);
}

bool UWeaponRangedAttackAbilityBase::ResolveViewData(
    const FVector& FallbackLoc,
    const FRotator& FallbackRot,
    bool bUseControllerViewRotation,
    float SpreadHalfAngleDeg,
    FVector& OutViewLoc,
    FVector& OutViewDir
) const
{
    OutViewLoc = FallbackLoc;
    OutViewDir = FallbackRot.Vector().GetSafeNormal();

    if (bUseControllerViewRotation)
    {
        FVector ViewLoc;
        FRotator ViewRot;
        if (GetViewPoint(ViewLoc, ViewRot))
        {
            OutViewLoc = ViewLoc;
            OutViewDir = ViewRot.Vector().GetSafeNormal();
        }
    }

    if (OutViewDir.IsNearlyZero())
    {
        OutViewDir = FallbackRot.Vector().GetSafeNormal();
    }

    if (SpreadHalfAngleDeg > 0.f)
    {
        const float HalfAngleRad = FMath::DegreesToRadians(SpreadHalfAngleDeg);
        OutViewDir = FMath::VRandCone(OutViewDir, HalfAngleRad).GetSafeNormal();
    }

    return !OutViewDir.IsNearlyZero();
}

bool UWeaponRangedAttackAbilityBase::ComputeAimPointFromView(
    const FVector& ViewLoc,
    const FVector& ViewDir,
    float TraceDistance,
    ECollisionChannel TraceChannel,
    bool bTraceComplex,
    float TraceRadius,
    FVector& OutAimPoint,
    FHitResult* OutViewHit
) const
{
    OutAimPoint = FVector::ZeroVector;

    AActor* Avatar = GetAvatarActorFromActorInfo();
    UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
    if (!World)
    {
        return false;
    }

    const float FinalDistance = FMath::Max(TraceDistance, 1.f);
    const FVector TraceEnd = ViewLoc + ViewDir * FinalDistance;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponRangedAimTrace), bTraceComplex);
    BuildTraceParams(Params, bTraceComplex);

    FHitResult LocalHit;
    bool bBlockingHit = false;

    if (TraceRadius > KINDA_SMALL_NUMBER)
    {
        const FCollisionShape Shape = FCollisionShape::MakeSphere(TraceRadius);
        bBlockingHit = World->SweepSingleByChannel(
            LocalHit,
            ViewLoc,
            TraceEnd,
            FQuat::Identity,
            TraceChannel,
            Shape,
            Params
        );
    }
    else
    {
        bBlockingHit = World->LineTraceSingleByChannel(
            LocalHit,
            ViewLoc,
            TraceEnd,
            TraceChannel,
            Params
        );
    }

    OutAimPoint = bBlockingHit ? LocalHit.ImpactPoint : TraceEnd;

    if (OutViewHit)
    {
        *OutViewHit = LocalHit;
    }

    return true;
}

bool UWeaponRangedAttackAbilityBase::ComputeShotDirectionFromAimPoint(
    const FVector& MuzzleLoc,
    const FVector& AimPoint,
    const FVector& FallbackDir,
    FVector& OutShotDirection
) const
{
    OutShotDirection = (AimPoint - MuzzleLoc).GetSafeNormal();

    if (OutShotDirection.IsNearlyZero())
    {
        OutShotDirection = FallbackDir.GetSafeNormal();
    }

    return !OutShotDirection.IsNearlyZero();
}

bool UWeaponRangedAttackAbilityBase::ComputeFinalHitscanHit(
    ARangedWeaponBase* Weapon,
    const FRangedHitscanConfig& Hitscan,
    FHitResult& OutFinalHit,
    FVector& OutAimPoint
) const
{
    OutFinalHit = FHitResult();
    OutAimPoint = FVector::ZeroVector;

    if (!Weapon)
    {
        return false;
    }

    FTransform MuzzleTf;
    if (!Weapon->GetWeaponSocketTransform(Hitscan.MuzzleSocket, MuzzleTf))
    {
        return false;
    }

    const FVector MuzzleLoc = MuzzleTf.GetLocation();

    FVector ViewLoc;
    FVector ViewDir;
    if (!ResolveViewData(
        MuzzleLoc,
        MuzzleTf.Rotator(),
        Hitscan.bUseControllerViewRotation,
        Hitscan.SpreadHalfAngleDeg,
        ViewLoc,
        ViewDir))
    {
        return false;
    }

    FHitResult ViewHit;
    if (!ComputeAimPointFromView(
        ViewLoc,
        ViewDir,
        Hitscan.MaxDistance,
        Hitscan.TraceChannel,
        Hitscan.bTraceComplex,
        Hitscan.Radius,
        OutAimPoint,
        &ViewHit))
    {
        return false;
    }

    FVector MuzzleDir;
    if (!ComputeShotDirectionFromAimPoint(MuzzleLoc, OutAimPoint, ViewDir, MuzzleDir))
    {
        return false;
    }

    const FVector MuzzleEnd = MuzzleLoc + MuzzleDir * Hitscan.MaxDistance;

    FHitResult MuzzleHit;
    const bool bMuzzleBlocking = TraceSingle(MuzzleLoc, MuzzleEnd, Hitscan, MuzzleHit);

    if (bDebugTrace)
    {
        AActor* Avatar = GetAvatarActorFromActorInfo();
        UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
        if (World)
        {
            const float Life = DebugLifeTime;
            const bool bViewBlocking = ViewHit.bBlockingHit;

            DrawDebugLine(
                World, ViewLoc, ViewLoc + ViewDir * Hitscan.MaxDistance,
                bViewBlocking ? FColor::Cyan : FColor::Silver,
                false, Life, 0, DebugLineThickness);

            DrawDebugLine(
                World, MuzzleLoc, MuzzleEnd,
                bMuzzleBlocking ? FColor::Yellow : FColor::Silver,
                false, Life, 0, DebugLineThickness);

            if (bViewBlocking)
            {
                DrawDebugSphere(World, ViewHit.ImpactPoint, 6.f, 12, FColor::Cyan, false, Life);
            }

            if (bMuzzleBlocking)
            {
                DrawDebugSphere(World, MuzzleHit.ImpactPoint, 7.f, 12, FColor::Red, false, Life);
            }
        }
    }

    if (!bMuzzleBlocking)
    {
        return false;
    }

    OutFinalHit = MuzzleHit;
    return true;
}

void UWeaponRangedAttackAbilityBase::HandleHitscanImpact(ARangedWeaponBase* Weapon, const FHitResult& FinalHit)
{
    if (!Weapon)
    {
        return;
    }

    if (!FinalHit.bBlockingHit &&
        FinalHit.ImpactPoint.IsNearlyZero() &&
        FinalHit.Location.IsNearlyZero())
    {
        return;
    }

    AActor* Avatar = GetAvatarActorFromActorInfo();
    AActor* HitActor = FinalHit.GetActor();

    if (HitActor == Avatar || HitActor == Weapon)
    {
        return;
    }

    SpawnWeaponHitImpactFXFromHitResult(FinalHit);

    if (!HitActor)
    {
        return;
    }

    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);

    if (!TargetASC)
    {
        return;
    }

    ApplyRangedOnHitEffects(HitActor);
}

bool UWeaponRangedAttackAbilityBase::TryGetMuzzleFlashTransform(
    ARangedWeaponBase* Weapon,
    FTransform& OutSpawnTransform
) const
{
    OutSpawnTransform = FTransform::Identity;

    if (!Weapon || !bHasCachedProfile)
    {
        return false;
    }

    if (!CachedProfile.MuzzleFlash.NiagaraSystem)
    {
        return false;
    }

    const FName MuzzleSocket =
        CachedProfile.IsProjectileMode()
        ? CachedProfile.Projectile.MuzzleSocket
        : CachedProfile.Hitscan.MuzzleSocket;

    FTransform SocketTransform;
    if (!Weapon->GetWeaponSocketTransform(MuzzleSocket, SocketTransform))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[RangedGA] MuzzleFlash socket transform failed. Socket=%s Weapon=%s"),
            *MuzzleSocket.ToString(),
            *GetNameSafe(Weapon));
        return false;
    }

    const FVector SpawnLocation =
        SocketTransform.TransformPosition(CachedProfile.MuzzleFlash.LocationOffset);

    const FRotator SpawnRotation =
        SocketTransform.Rotator() + CachedProfile.MuzzleFlash.RotationOffset;

    OutSpawnTransform = FTransform(
        SpawnRotation,
        SpawnLocation,
        CachedProfile.MuzzleFlash.Scale
    );

    return true;
}

bool UWeaponRangedAttackAbilityBase::SpawnMuzzleFlash(ARangedWeaponBase* Weapon) const
{
    if (!Weapon || !bHasCachedProfile)
    {
        return false;
    }

    const auto& MuzzleFlash = CachedProfile.MuzzleFlash;
    if (!MuzzleFlash.NiagaraSystem)
    {
        return false;
    }

    UWorld* World = Weapon->GetWorld();
    if (!World)
    {
        return false;
    }

    FTransform SpawnTransform;
    if (!TryGetMuzzleFlashTransform(Weapon, SpawnTransform))
    {
        return false;
    }

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        MuzzleFlash.NiagaraSystem,
        SpawnTransform.GetLocation(),
        SpawnTransform.Rotator(),
        SpawnTransform.GetScale3D(),
        true,
        true
    );

    return true;
}

void UWeaponRangedAttackAbilityBase::FireCurrentProfile(ARangedWeaponBase* Weapon)
{
    if (!Weapon || !bHasCachedProfile)
    {
        return;
    }

    SpawnMuzzleFlash(Weapon);

    if (CachedProfile.IsProjectileMode())
    {
        FireProjectileOnce(Weapon);
        return;
    }

    FireHitscanOnce(Weapon);
}

bool UWeaponRangedAttackAbilityBase::TryGetProjectileSpawnTransform(
    ARangedWeaponBase* Weapon,
    FTransform& OutSpawnTransform,
    FVector& OutShotDirection
) const
{
    OutSpawnTransform = FTransform::Identity;
    OutShotDirection = FVector::ForwardVector;

    if (!Weapon || !bHasCachedProfile)
    {
        return false;
    }

    const FRangedProjectileConfig& Projectile = CachedProfile.Projectile;
    if (!Projectile.ProjectileClass)
    {
        return false;
    }

    FTransform MuzzleTf;
    if (!Weapon->GetWeaponSocketTransform(Projectile.MuzzleSocket, MuzzleTf))
    {
        return false;
    }

    const FVector SpawnLoc = MuzzleTf.TransformPosition(Projectile.SpawnOffset);
    const FVector MuzzleForward = MuzzleTf.GetRotation().GetForwardVector().GetSafeNormal();

    FVector ViewLoc;
    FVector ViewDir;
    if (!ResolveViewData(
        SpawnLoc,
        MuzzleTf.Rotator(),
        Projectile.bUseControllerViewRotation,
        0.f,
        ViewLoc,
        ViewDir))
    {
        return false;
    }

    const float AimDistance =
        (CachedProfile.Hitscan.MaxDistance > 0.f) ? CachedProfile.Hitscan.MaxDistance : 10000.f;

    const ECollisionChannel AimChannel = CachedProfile.Hitscan.TraceChannel;
    const bool bAimTraceComplex = CachedProfile.Hitscan.bTraceComplex;

    FHitResult ViewHit;
    FVector AimPoint;
    if (!ComputeAimPointFromView(
        ViewLoc,
        ViewDir,
        AimDistance,
        AimChannel,
        bAimTraceComplex,
        0.f,
        AimPoint,
        &ViewHit))
    {
        return false;
    }

    if (!ComputeShotDirectionFromAimPoint(SpawnLoc, AimPoint, MuzzleForward, OutShotDirection))
    {
        return false;
    }

    OutSpawnTransform = FTransform(OutShotDirection.Rotation(), SpawnLoc, FVector::OneVector);

    if (bDebugTrace)
    {
        UWorld* World = Weapon->GetWorld();
        if (World)
        {
            const float Life = DebugLifeTime;

            DrawDebugLine(
                World,
                ViewLoc,
                ViewLoc + ViewDir * AimDistance,
                ViewHit.bBlockingHit ? FColor::Green : FColor::Silver,
                false,
                Life,
                0,
                DebugLineThickness
            );

            DrawDebugLine(
                World,
                SpawnLoc,
                SpawnLoc + OutShotDirection * 300.f,
                FColor::Orange,
                false,
                Life,
                0,
                DebugLineThickness
            );

            DrawDebugSphere(World, SpawnLoc, 4.f, 8, FColor::Orange, false, Life);

            if (ViewHit.bBlockingHit)
            {
                DrawDebugSphere(World, ViewHit.ImpactPoint, 6.f, 12, FColor::Green, false, Life);
            }
        }
    }

    return true;
}

void UWeaponRangedAttackAbilityBase::FireProjectileOnce(ARangedWeaponBase* Weapon)
{
    UE_LOG(LogTemp, Warning,
        TEXT("[RangedGA] Base FireProjectileOnce should not be used directly. Weapon=%s Ability=%s"),
        *GetNameSafe(Weapon),
        *GetNameSafe(this));
}

bool UWeaponRangedAttackAbilityBase::ApplyProjectileLaunchSettings(
    AActor* SpawnedProjectile,
    const FVector& ShotDirection
) const
{
    if (!SpawnedProjectile || !bHasCachedProfile)
    {
        return false;
    }

    const FRangedProjectileConfig& Projectile = CachedProfile.Projectile;

    const bool bHasSpeedMode = Projectile.HasValidSpeedMode();
    const bool bHasImpulseMode = Projectile.HasValidImpulseMode();

    if (!bHasSpeedMode && !bHasImpulseMode)
    {
        return false;
    }

    if (Projectile.LifeSeconds > 0.f)
    {
        SpawnedProjectile->SetLifeSpan(Projectile.LifeSeconds);
    }

    FVector LaunchDir = ShotDirection.GetSafeNormal();
    if (LaunchDir.IsNearlyZero())
    {
        LaunchDir = SpawnedProjectile->GetActorForwardVector().GetSafeNormal();
    }

    if (LaunchDir.IsNearlyZero())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[RangedGA] Launch direction is invalid. Projectile=%s"),
            *GetNameSafe(SpawnedProjectile));
        return false;
    }

    UProjectileMovementComponent* MoveComp =
        SpawnedProjectile->FindComponentByClass<UProjectileMovementComponent>();

    UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(SpawnedProjectile->GetRootComponent());
    if (!Prim)
    {
        Prim = SpawnedProjectile->FindComponentByClass<UPrimitiveComponent>();
    }

    AProjectileBase* ProjectileActor = Cast<AProjectileBase>(SpawnedProjectile);

    // 수정:
    // BP에서 Impulse를 체크한 경우 기본 Speed 값이 살아 있어도
    // 사용자가 의도한 대로 Impulse를 우선 적용한다.
    if (bHasImpulseMode)
    {
        if (bHasSpeedMode)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[RangedGA] Both Speed and Impulse are valid. Impulse will be preferred. Projectile=%s"),
                *GetNameSafe(SpawnedProjectile));
        }

        if (MoveComp)
        {
            MoveComp->StopMovementImmediately();
            MoveComp->Deactivate();
            MoveComp->Velocity = FVector::ZeroVector;
            MoveComp->ProjectileGravityScale = 0.f;
        }

        if (!Prim)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[RangedGA] Impulse mode requested but no PrimitiveComponent found. Projectile=%s"),
                *GetNameSafe(SpawnedProjectile));
            return false;
        }

        // 수정: 임펄스 모드는 코드에서 직접 물리 on
        Prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Prim->SetSimulatePhysics(true);
        Prim->SetEnableGravity(Projectile.GravityScale > 0.f);
        Prim->WakeAllRigidBodies();
        Prim->SetPhysicsLinearVelocity(FVector::ZeroVector);
        Prim->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

        if (!Prim->IsSimulatingPhysics())
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[RangedGA] Failed to enable physics for impulse mode. Projectile=%s Component=%s"),
                *GetNameSafe(SpawnedProjectile),
                *GetNameSafe(Prim));
            return false;
        }

        if (ProjectileActor)
        {
            ProjectileActor->ConfigureImpulsePhysics(Prim, Projectile.GravityScale);
        }

        Prim->AddImpulse(LaunchDir * Projectile.LaunchImpulse, NAME_None, true);
        return true;
    }

    if (bHasSpeedMode)
    {
        if (!MoveComp)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[RangedGA] Speed mode requested but ProjectileMovementComponent missing. Projectile=%s"),
                *GetNameSafe(SpawnedProjectile));
            return false;
        }

        // 수정: speed 모드는 물리 시뮬레이션 off
        if (Prim && Prim->IsSimulatingPhysics())
        {
            Prim->SetPhysicsLinearVelocity(FVector::ZeroVector);
            Prim->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
            Prim->SetSimulatePhysics(false);
        }

        if (ProjectileActor)
        {
            ProjectileActor->ConfigureImpulsePhysics(nullptr, 0.f);
        }

        MoveComp->StopMovementImmediately();
        MoveComp->Velocity = FVector::ZeroVector;
        MoveComp->InitialSpeed = Projectile.InitialSpeed;
        MoveComp->MaxSpeed = Projectile.MaxSpeed;
        MoveComp->ProjectileGravityScale = Projectile.GravityScale;
        MoveComp->Velocity = LaunchDir * Projectile.InitialSpeed;
        MoveComp->Activate(true);
        return true;
    }

    return false;
}

void UWeaponRangedAttackAbilityBase::FireHitscanOnce(ARangedWeaponBase* Weapon)
{
    if (!Weapon || !bHasCachedProfile)
    {
        return;
    }

    const FRangedHitscanConfig& Hitscan = CachedProfile.Hitscan;
    const int32 Shots = FMath::Max(1, Hitscan.NumShots);

    for (int32 i = 0; i < Shots; ++i)
    {
        FHitResult FinalHit;
        FVector AimPoint;

        const bool bHit = ComputeFinalHitscanHit(Weapon, Hitscan, FinalHit, AimPoint);
        if (!bHit)
        {
            continue;
        }

        HandleHitscanImpact(Weapon, FinalHit);
    }
}

bool UWeaponRangedAttackAbilityBase::ApplyRangedOnHitEffects(AActor* TargetActor) const
{
    if (!TargetActor || !bHasCachedProfile)
    {
        return false;
    }

    ARangedWeaponBase* Weapon = GetWeaponFromSourceObject<ARangedWeaponBase>();
    if (!Weapon)
    {
        return false;
    }

    bool bAnyApplied = false;

    float FinalDamage = 0.f;
    if (TryGetCachedFinalDamage(Weapon, FinalDamage) && FinalDamage > 0.f)
    {
        bAnyApplied |= ApplyBaseDamageToTargetActor(TargetActor, FinalDamage, 1.f, 1.f);
    }

    const FGameplayTag DamageTag = GetDataDamageTag();

    for (const FRangedOnHitGameplayEffectSpec& Spec : CachedProfile.OnHitTargetEffects)
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

void UWeaponRangedAttackAbilityBase::OnFireEventReceived(FGameplayEventData Payload)
{
    ARangedWeaponBase* Weapon = GetWeaponFromSourceObject<ARangedWeaponBase>();
    if (!Weapon || !bHasCachedProfile)
    {
        return;
    }

    FireCurrentProfile(Weapon);
}

void UWeaponRangedAttackAbilityBase::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UWeaponRangedAttackAbilityBase::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UWeaponRangedAttackAbilityBase::OnMontageInterrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UWeaponRangedAttackAbilityBase::OnMontageBlendOut()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}