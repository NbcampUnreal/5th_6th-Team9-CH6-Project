// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/GameAbility/RangedAbility/WeaponRangedAttackAbilityBase.h"

#include "Weapons/RangedWeapon/RangedWeaponBase.h"
#include "Weapons/WeaponBase.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"

#include "DrawDebugHelpers.h"

UWeaponRangedAttackAbilityBase::UWeaponRangedAttackAbilityBase()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    FireTag = FGameplayTag::RequestGameplayTag(TEXT("Attack.Primary"), false);
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
    if (!Weapon || !FireTag.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }

    if (!CacheFireProfileFromWeapon(Weapon))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }

    // 발사 이벤트 대기 (몽타주 타이밍)
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

    // 몽타주가 있으면 재생, 없으면 즉시 발사 후 종료
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

        // 몽타주가 있는데 FireEventTag가 유효하지 않으면 즉시 1회 발사 옵션
        if (bFireImmediatelyIfNoMontageOrEvent && !FireEventTag.IsValid())
        {
            FireHitscanOnce(Weapon);
        }

        return;
    }

    if (bFireImmediatelyIfNoMontageOrEvent)
    {
        FireHitscanOnce(Weapon);
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
    if (!Weapon) return false;

    FRangedFireProfile Profile;
    if (!Weapon->GetFireProfile(FireTag, Profile))
    {
        UE_LOG(LogTemp, Warning, TEXT("[RangedGA] GetFireProfile failed FireTag=%s Weapon=%s"),
            *FireTag.ToString(), *GetNameSafe(Weapon));
        return false;
    }

    CachedProfile = Profile;
    bHasCachedProfile = true;
    return true;
}

bool UWeaponRangedAttackAbilityBase::GetViewPoint(FVector& OutLoc, FRotator& OutRot) const
{
    const FGameplayAbilityActorInfo* Info = CurrentActorInfo;
    if (!Info) return false;

    APawn* Pawn = Cast<APawn>(Info->AvatarActor.Get());
    AController* Controller = Info->PlayerController.Get();

    if (!Controller && Pawn)
    {
        Controller = Pawn->GetController();
    }

    // 플레이어: 카메라 매니저 기준 ViewPoint
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        PC->GetPlayerViewPoint(OutLoc, OutRot);
        return true;
    }

    // AI/기타: PawnViewLocation + ControlRotation
    if (Pawn && Controller)
    {
        OutLoc = Pawn->GetPawnViewLocation();
        OutRot = Controller->GetControlRotation();
        return true;
    }

    // fallback
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
    if (!World) return false;

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

bool UWeaponRangedAttackAbilityBase::ComputeFinalHitscanHit(
    ARangedWeaponBase* Weapon,
    const FRangedHitscanConfig& Hitscan,
    FHitResult& OutFinalHit,
    FVector& OutAimPoint
) const
{
    OutFinalHit = FHitResult();
    OutAimPoint = FVector::ZeroVector;

    if (!Weapon) return false;

    // 1) 총구 트랜스폼
    FTransform MuzzleTf;
    Weapon->GetWeaponSocketTransform(Hitscan.MuzzleSocket, MuzzleTf);
    const FVector MuzzleLoc = MuzzleTf.GetLocation();

    // 2) ViewPoint(카메라/컨트롤러) 또는 총구 기준
    FVector ViewLoc;
    FRotator ViewRot;

    if (Hitscan.bUseControllerViewRotation)
    {
        if (!GetViewPoint(ViewLoc, ViewRot))
        {
            ViewLoc = MuzzleLoc;
            ViewRot = MuzzleTf.Rotator();
        }
    }
    else
    {
        ViewLoc = MuzzleLoc;
        ViewRot = MuzzleTf.Rotator();
    }

    // 3) 스프레드 적용
    FVector ViewDir = ViewRot.Vector();
    if (Hitscan.SpreadHalfAngleDeg > 0.f)
    {
        const float HalfAngleRad = FMath::DegreesToRadians(Hitscan.SpreadHalfAngleDeg);
        ViewDir = FMath::VRandCone(ViewDir, HalfAngleRad);
    }

    const FVector ViewEnd = ViewLoc + ViewDir * Hitscan.MaxDistance;

    // 1차: 카메라 트레이스 -> AimPoint
    FHitResult ViewHit;
    const bool bViewBlocking = TraceSingle(ViewLoc, ViewEnd, Hitscan, ViewHit);

    OutAimPoint = bViewBlocking ? ViewHit.ImpactPoint : ViewEnd;

    // 2차: 총구 -> AimPoint 방향으로 최종 트레이스 (총구 앞 벽 먼저 맞게)
    FVector MuzzleDir = (OutAimPoint - MuzzleLoc).GetSafeNormal();
    if (MuzzleDir.IsNearlyZero())
    {
        MuzzleDir = ViewDir;
    }

    const FVector MuzzleEnd = MuzzleLoc + MuzzleDir * Hitscan.MaxDistance;

    FHitResult MuzzleHit;
    const bool bMuzzleBlocking = TraceSingle(MuzzleLoc, MuzzleEnd, Hitscan, MuzzleHit);

    // 디버그
    if (bDebugTrace)
    {
        AActor* Avatar = GetAvatarActorFromActorInfo();
        UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
        if (World)
        {
            const float Life = DebugLifeTime;

            DrawDebugLine(World, ViewLoc, ViewEnd,
                bViewBlocking ? FColor::Cyan : FColor::Silver,
                false, Life, 0, DebugLineThickness);

            DrawDebugLine(World, MuzzleLoc, MuzzleEnd,
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

    if (bMuzzleBlocking)
    {
        OutFinalHit = MuzzleHit;
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
        if (!bHit) continue;

        AActor* HitActor = FinalHit.GetActor();
        if (!HitActor) continue;

        AActor* Avatar = GetAvatarActorFromActorInfo();
        if (HitActor == Avatar || HitActor == Weapon) continue;

        ApplyRangedOnHitEffects(HitActor);
    }
}

bool UWeaponRangedAttackAbilityBase::ApplyRangedOnHitEffects(AActor* TargetActor) const
{
    if (!TargetActor || !bHasCachedProfile) return false;

    bool bAnyApplied = false;

    // BaseDamage
    if (CachedProfile.BaseDamage > 0.f)
    {
        bAnyApplied |= ApplyBaseDamageToTargetActor(TargetActor, CachedProfile.BaseDamage, 1.f, 1.f);
    }

    // OnHit effects
    for (const FRangedOnHitGameplayEffectSpec& Spec : CachedProfile.OnHitTargetEffects)
    {
        if (!Spec.Effect) continue;

        bAnyApplied |= ApplyEffectToTargetActor(
            TargetActor,
            Spec.Effect,
            Spec.Level,
            Spec.SetByCallerMagnitudes,
            Spec.Chance
        );
    }

    return bAnyApplied;
}

void UWeaponRangedAttackAbilityBase::OnFireEventReceived(FGameplayEventData Payload)
{
    ARangedWeaponBase* Weapon = GetWeaponFromSourceObject<ARangedWeaponBase>();
    if (!Weapon || !bHasCachedProfile) return;

    FireHitscanOnce(Weapon);
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
    // 필요 시 연사/후처리 확장
}
