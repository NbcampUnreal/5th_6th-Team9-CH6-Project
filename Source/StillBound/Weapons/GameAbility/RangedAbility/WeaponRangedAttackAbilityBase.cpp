#include "Weapons/GameAbility/RangedAbility/WeaponRangedAttackAbilityBase.h"

#include "Weapons/RangedWeapon/RangedWeaponBase.h"
#include "Weapons/RangedWeapon/ProjectileBase.h"
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

    AimTraceStartForwardOffset = 50.0f;
    MinForwardDotForAimCandidate = 0.05f;
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

        MontageTask->OnCompleted.AddDynamic(this, &UWeaponRangedAttackAbilityBase::OnMontageCompleted);
        MontageTask->OnCancelled.AddDynamic(this, &UWeaponRangedAttackAbilityBase::OnMontageCancelled);
        MontageTask->OnInterrupted.AddDynamic(this, &UWeaponRangedAttackAbilityBase::OnMontageInterrupted);
        MontageTask->OnBlendOut.AddDynamic(this, &UWeaponRangedAttackAbilityBase::OnMontageBlendOut);
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

bool UWeaponRangedAttackAbilityBase::TraceAimCandidatesFromView(
    const FVector& ViewLoc,
    const FVector& ViewDir,
    float TraceDistance,
    ECollisionChannel TraceChannel,
    bool bTraceComplex,
    float TraceRadius,
    TArray<FHitResult>& OutHits
) const
{
    OutHits.Reset();

    AActor* Avatar = GetAvatarActorFromActorInfo();
    UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
    if (!World)
    {
        return false;
    }

    const FVector SafeViewDir = ViewDir.GetSafeNormal();
    if (SafeViewDir.IsNearlyZero())
    {
        return false;
    }

    const float FinalDistance = FMath::Max(TraceDistance, 1.f);
    const FVector TraceStart = ViewLoc + SafeViewDir * FMath::Max(0.f, AimTraceStartForwardOffset);
    const FVector TraceEnd = TraceStart + SafeViewDir * FinalDistance;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponRangedAimTrace), bTraceComplex);
    BuildTraceParams(Params, bTraceComplex);

    bool bAnyHit = false;

    if (TraceRadius > KINDA_SMALL_NUMBER)
    {
        const FCollisionShape Shape = FCollisionShape::MakeSphere(TraceRadius);
        bAnyHit = World->SweepMultiByChannel(
            OutHits,
            TraceStart,
            TraceEnd,
            FQuat::Identity,
            TraceChannel,
            Shape,
            Params
        );
    }
    else
    {
        bAnyHit = World->LineTraceMultiByChannel(
            OutHits,
            TraceStart,
            TraceEnd,
            TraceChannel,
            Params
        );
    }

    if (bAnyHit && OutHits.Num() > 1)
    {
        auto GetSortPoint = [](const FHitResult& Hit) -> FVector
            {
                if (!Hit.ImpactPoint.IsNearlyZero())
                {
                    return Hit.ImpactPoint;
                }

                if (!Hit.Location.IsNearlyZero())
                {
                    return Hit.Location;
                }

                return Hit.TraceEnd;
            };

        OutHits.Sort([&TraceStart, &GetSortPoint](const FHitResult& A, const FHitResult& B)
            {
                return FVector::DistSquared(TraceStart, GetSortPoint(A))
                    < FVector::DistSquared(TraceStart, GetSortPoint(B));
            });
    }

    return true;
}

bool UWeaponRangedAttackAbilityBase::IsForwardAimCandidate(
    const FVector& MuzzleLoc,
    const FVector& MuzzleForward,
    const FVector& CandidatePoint,
    float MinDotThreshold
) const
{
    const FVector SafeMuzzleForward = MuzzleForward.GetSafeNormal();
    if (SafeMuzzleForward.IsNearlyZero())
    {
        return true;
    }

    const FVector ToCandidate = (CandidatePoint - MuzzleLoc).GetSafeNormal();
    if (ToCandidate.IsNearlyZero())
    {
        return false;
    }

    const float Dot = FVector::DotProduct(SafeMuzzleForward, ToCandidate);
    return Dot > MinDotThreshold;
}

bool UWeaponRangedAttackAbilityBase::ComputeAimPointFromView(
    const FVector& ViewLoc,
    const FVector& ViewDir,
    float TraceDistance,
    ECollisionChannel TraceChannel,
    bool bTraceComplex,
    float TraceRadius,
    const FVector& MuzzleLoc,
    const FVector& MuzzleForward,
    FVector& OutAimPoint,
    FHitResult* OutViewHit
) const
{
    OutAimPoint = FVector::ZeroVector;

    const FVector SafeViewDir = ViewDir.GetSafeNormal();
    if (SafeViewDir.IsNearlyZero())
    {
        return false;
    }

    const float FinalDistance = FMath::Max(TraceDistance, 1.f);
    const FVector TraceStart = ViewLoc + SafeViewDir * FMath::Max(0.f, AimTraceStartForwardOffset);
    const FVector TraceEnd = TraceStart + SafeViewDir * FinalDistance;

    TArray<FHitResult> ViewHits;
    if (!TraceAimCandidatesFromView(
        ViewLoc,
        ViewDir,
        TraceDistance,
        TraceChannel,
        bTraceComplex,
        TraceRadius,
        ViewHits))
    {
        return false;
    }

    auto GetCandidatePoint = [](const FHitResult& Hit) -> FVector
        {
            if (!Hit.ImpactPoint.IsNearlyZero())
            {
                return Hit.ImpactPoint;
            }

            if (!Hit.Location.IsNearlyZero())
            {
                return Hit.Location;
            }

            return Hit.TraceEnd;
        };

    for (const FHitResult& Hit : ViewHits)
    {
        if (!Hit.bBlockingHit)
        {
            continue;
        }

        const FVector CandidatePoint = GetCandidatePoint(Hit);

        if (!IsForwardAimCandidate(
            MuzzleLoc,
            MuzzleForward,
            CandidatePoint,
            MinForwardDotForAimCandidate))
        {
            continue;
        }

        OutAimPoint = CandidatePoint;

        if (OutViewHit)
        {
            *OutViewHit = Hit;
        }

        return true;
    }

    if (IsForwardAimCandidate(MuzzleLoc, MuzzleForward, TraceEnd, 0.f))
    {
        OutAimPoint = TraceEnd;
    }
    else
    {
        const FVector SafeMuzzleForward = MuzzleForward.GetSafeNormal();
        if (SafeMuzzleForward.IsNearlyZero())
        {
            OutAimPoint = TraceEnd;
        }
        else
        {
            OutAimPoint = MuzzleLoc + SafeMuzzleForward * FinalDistance;
        }
    }

    if (OutViewHit)
    {
        *OutViewHit = FHitResult();
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
    const FVector MuzzleForward = MuzzleTf.GetRotation().GetForwardVector().GetSafeNormal();

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
        MuzzleLoc,
        MuzzleForward,
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
            const FVector DebugViewStart = ViewLoc + ViewDir.GetSafeNormal() * FMath::Max(0.f, AimTraceStartForwardOffset);
            const FVector DebugViewEnd = DebugViewStart + ViewDir.GetSafeNormal() * Hitscan.MaxDistance;
            const bool bViewBlocking = ViewHit.bBlockingHit;

            DrawDebugLine(
                World, DebugViewStart, DebugViewEnd,
                bViewBlocking ? FColor::Cyan : FColor::Silver,
                false, Life, 0, DebugLineThickness);

            DrawDebugLine(
                World, MuzzleLoc, MuzzleEnd,
                bMuzzleBlocking ? FColor::Yellow : FColor::Silver,
                false, Life, 0, DebugLineThickness);

            // [추가] 총구 소켓의 실제 Forward 방향 확인용
            DrawDebugLine(
                World,
                MuzzleLoc,
                MuzzleLoc + MuzzleForward * 100.f,
                FColor::Magenta,
                false,
                Life,
                0,
                2.f
            );

            DrawDebugSphere(World, OutAimPoint, 5.f, 12, FColor::Blue, false, Life);

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
        SpawnLoc,
        MuzzleForward,
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
            const FVector DebugViewStart = ViewLoc + ViewDir.GetSafeNormal() * FMath::Max(0.f, AimTraceStartForwardOffset);
            const FVector DebugViewEnd = DebugViewStart + ViewDir.GetSafeNormal() * AimDistance;

            DrawDebugLine(
                World,
                DebugViewStart,
                DebugViewEnd,
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

            // [추가] 총구 소켓의 실제 Forward 방향 확인용
            DrawDebugLine(
                World,
                SpawnLoc,
                SpawnLoc + MuzzleForward * 100.f,
                FColor::Magenta,
                false,
                Life,
                0,
                2.f
            );

            DrawDebugSphere(World, SpawnLoc, 4.f, 8, FColor::Orange, false, Life);
            DrawDebugSphere(World, AimPoint, 5.f, 12, FColor::Blue, false, Life);

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

    if (bHasSpeedMode && bHasImpulseMode)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[RangedGA] Both Speed and Impulse are valid. Speed will be preferred. Projectile=%s"),
            *GetNameSafe(SpawnedProjectile));
    }

    if (Projectile.LifeSeconds > 0.f)
    {
        SpawnedProjectile->SetLifeSpan(Projectile.LifeSeconds);
    }

    if (bHasSpeedMode)
    {
        if (UProjectileMovementComponent* MoveComp =
            SpawnedProjectile->FindComponentByClass<UProjectileMovementComponent>())
        {
            const FVector SafeShotDir = ShotDirection.GetSafeNormal();

            MoveComp->StopMovementImmediately();
            MoveComp->InitialSpeed = Projectile.InitialSpeed;
            MoveComp->MaxSpeed = Projectile.MaxSpeed;
            MoveComp->ProjectileGravityScale = Projectile.bEnableGravity ? 1.f : 0.f;
            MoveComp->SetVelocityInLocalSpace(FVector(Projectile.InitialSpeed, 0.f, 0.f));

            if (!SafeShotDir.IsNearlyZero())
            {
                SpawnedProjectile->SetActorRotation(SafeShotDir.Rotation());
            }

            MoveComp->Activate(true);
            MoveComp->UpdateComponentVelocity();
            return true;
        }

        UE_LOG(LogTemp, Warning,
            TEXT("[RangedGA] Speed mode requested but ProjectileMovementComponent missing. Projectile=%s"),
            *GetNameSafe(SpawnedProjectile));
        return false;
    }

    if (bHasImpulseMode)
    {
        UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(SpawnedProjectile->GetRootComponent());
        if (!Prim)
        {
            Prim = SpawnedProjectile->FindComponentByClass<UPrimitiveComponent>();
        }

        if (!Prim)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[RangedGA] Impulse mode requested but no PrimitiveComponent found. Projectile=%s"),
                *GetNameSafe(SpawnedProjectile));
            return false;
        }

        if (!Prim->IsSimulatingPhysics())
        {
            Prim->SetSimulatePhysics(true);
            Prim->WakeAllRigidBodies();
        }

        if (!Prim->IsSimulatingPhysics())
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[RangedGA] Failed to enable physics for impulse projectile. Projectile=%s Component=%s"),
                *GetNameSafe(SpawnedProjectile),
                *GetNameSafe(Prim));
            return false;
        }

        if (AProjectileBase* ProjectileActor = Cast<AProjectileBase>(SpawnedProjectile))
        {
            ProjectileActor->ConfigureImpulsePhysics(Prim, Projectile.bEnableGravity);
        }
        else
        {
            Prim->SetEnableGravity(Projectile.bEnableGravity);
        }

        Prim->SetPhysicsLinearVelocity(FVector::ZeroVector);
        Prim->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

        const FVector SafeShotDir = ShotDirection.GetSafeNormal();
        if (!SafeShotDir.IsNearlyZero())
        {
            SpawnedProjectile->SetActorRotation(SafeShotDir.Rotation());
        }

        Prim->WakeAllRigidBodies();
        Prim->AddImpulse(SafeShotDir * Projectile.LaunchImpulse, NAME_None, true);
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
}