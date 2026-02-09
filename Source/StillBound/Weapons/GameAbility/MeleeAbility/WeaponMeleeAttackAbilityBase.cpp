// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GameAbility/MeleeAbility/WeaponMeleeAttackAbilityBase.h"

#include "Weapons/MeleeWeaponBase/MeleeWeaponBase.h"
#include "Weapons/WeaponBase.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"



UWeaponMeleeAttackAbilityBase::UWeaponMeleeAttackAbilityBase()
{
    // 싱글플레이: LocalOnly 유지
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // 기본 태그(파생에서 덮어써도 됨)
    AttackTag = FGameplayTag::RequestGameplayTag(TEXT("Attack.Light"), false);
}

void UWeaponMeleeAttackAbilityBase::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData
)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    AMeleeWeaponBase* Weapon = GetWeaponFromSourceObject<AMeleeWeaponBase>();
    if (!Weapon || !AttackTag.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }

    if (!CacheProfileFromWeapon(Weapon))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }

    HitActors.Reset();

    // 몽타주 없으면: 1회 스윕만 하고 종료 (테스트/프로토타입에 유용)
    if (!CachedProfile.Montage)
    {
        DoSweepTick();
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
        return;
    }

    MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this,
        NAME_None,
        CachedProfile.Montage,
        CachedProfile.MontagePlayRate,
        NAME_None,
        true
    );

    if (!MontageTask)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }

    MontageTask->OnCompleted.AddDynamic(this, &UWeaponMeleeAttackAbilityBase::OnMontageCompleted);
    MontageTask->OnCancelled.AddDynamic(this, &UWeaponMeleeAttackAbilityBase::OnMontageCancelled);
    MontageTask->OnInterrupted.AddDynamic(this, &UWeaponMeleeAttackAbilityBase::OnMontageInterrupted);
    MontageTask->OnBlendOut.AddDynamic(this, &UWeaponMeleeAttackAbilityBase::OnMontageBlendOut);

    MontageTask->ReadyForActivation();

    StartSweeping();
}

void UWeaponMeleeAttackAbilityBase::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled
)
{
    StopSweeping();
    HitActors.Reset();
    MontageTask = nullptr;

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UWeaponMeleeAttackAbilityBase::CacheProfileFromWeapon(AMeleeWeaponBase* Weapon)
{
    bHasCachedProfile = false;

    FWeaponAttackProfile Profile;
    if (!Weapon->GetAttackProfile(AttackTag, Profile))
    {
        return false;
    }

    CachedProfile = Profile;
    CachedSweep = Profile.Sweep;
    bHasCachedProfile = true;
    return true;
}

void UWeaponMeleeAttackAbilityBase::StartSweeping()
{
    if (!bHasCachedProfile)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        if (AActor* Avatar = GetAvatarActorFromActorInfo())
        {
            World = Avatar->GetWorld();
        }
    }
    if (!World)
    {
        return;
    }

    FTimerDelegate Del;
    Del.BindUObject(this, &UWeaponMeleeAttackAbilityBase::DoSweepTick);

    World->GetTimerManager().SetTimer(SweepTimerHandle, Del, SweepInterval, true);
}

void UWeaponMeleeAttackAbilityBase::StopSweeping()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        if (AActor* Avatar = GetAvatarActorFromActorInfo())
        {
            World = Avatar->GetWorld();
        }
    }
    if (World)
    {
        World->GetTimerManager().ClearTimer(SweepTimerHandle);
    }
}

void UWeaponMeleeAttackAbilityBase::DoSweepTick()
{
    if (!bHasCachedProfile)
    {
        return;
    }

    AActor* Avatar = GetAvatarActorFromActorInfo();
    AMeleeWeaponBase* Weapon = GetWeaponFromSourceObject<AMeleeWeaponBase>();
    if (!Avatar || !Weapon)
    {
        return;
    }

    USkeletalMeshComponent* WeaponMesh = Weapon->GetWeaponMesh();
    FVector Start = Avatar->GetActorLocation();
    FVector End = Start + Avatar->GetActorForwardVector() * CachedSweep.MaxDistance;

    if (WeaponMesh)
    {
        const bool bHasStartSocket = WeaponMesh->DoesSocketExist(CachedSweep.TraceStartSocket);
        const bool bHasEndSocket = WeaponMesh->DoesSocketExist(CachedSweep.TraceEndSocket);

        if (bHasStartSocket)
        {
            Start = WeaponMesh->GetSocketLocation(CachedSweep.TraceStartSocket);
        }
        if (bHasEndSocket)
        {
            End = WeaponMesh->GetSocketLocation(CachedSweep.TraceEndSocket);
        }
    }

    UWorld* World = Avatar->GetWorld();
    if (!World)
    {
        return;
    }
    // ? ADDED: 스윕 디버그 드로우 (Start/End 결정된 "직후"가 최적 위치)
    {
        // 히트 결과에 따라 색 바꾸고 싶으면 SweepMulti 후로 옮겨도 됨.
        const float LifeTime = 0.1f;        // 0.1~0.2 정도면 연타 시 궤적이 보임
        const float Thickness = 2.0f;

        // 1) 중심선
        DrawDebugLine(
            World,
            Start,
            End,
            FColor::Cyan,
            false,
            LifeTime,
            0,
            Thickness
        );

        // 2) 스윕 반지름(양 끝 구체)
        DrawDebugSphere(
            World,
            Start,
            CachedSweep.Radius,
            12,
            FColor::Cyan,
            false,
            LifeTime
        );

        DrawDebugSphere(
            World,
            End,
            CachedSweep.Radius,
            12,
            FColor::Cyan,
            false,
            LifeTime
        );

        // 3) 방향 화살표(선택)
        DrawDebugDirectionalArrow(
            World,
            Start,
            End,
            25.f,
            FColor::Cyan,
            false,
            LifeTime,
            0,
            Thickness
        );
    }


    FCollisionQueryParams Params(SCENE_QUERY_STAT(MeleeSweep), false);
    Params.AddIgnoredActor(Avatar);
    Params.AddIgnoredActor(Weapon);

    // 각 액터 1회만 때리기 옵션이면, 이미 맞은 액터는 무시
    if (CachedSweep.bHitEachActorOnce)
    {
        for (const TWeakObjectPtr<AActor>& HitA : HitActors)
        {
            if (HitA.IsValid())
            {
                Params.AddIgnoredActor(HitA.Get());
            }
        }
    }

    TArray<FHitResult> Hits;
    const FCollisionShape Shape = FCollisionShape::MakeSphere(CachedSweep.Radius);

    const bool bHit = World->SweepMultiByChannel(
        Hits,
        Start,
        End,
        FQuat::Identity,
        CachedSweep.TraceChannel,
        Shape,
        Params
    );

    if (!bHit)
    {
        return;
    }

    for (const FHitResult& HR : Hits)
    {
        AActor* HitActor = HR.GetActor();
        if (!HitActor || HitActor == Avatar || HitActor == Weapon)
        {
            continue;
        }

        // TSet<TWeakObjectPtr<AActor>>와 비교를 명확히 (암시 변환 의존 제거)
        if (CachedSweep.bHitEachActorOnce && HitActors.Contains(TWeakObjectPtr<AActor>(HitActor)))
        {
            continue;
        }

        // 온힛 적용
        const bool bApplied = ApplyOnHitEffects(HitActor);
        if (bApplied)
        {
            HitActors.Add(HitActor);

            if (CachedSweep.bHitFirstTargetOnly)
            {
                StopSweeping();
                break;
            }
        }
    }
}

bool UWeaponMeleeAttackAbilityBase::ApplyOnHitEffects(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return false;
    }

    bool bAnyApplied = false;

    // ✅ FIX: 베이스 데미지는 임시로 10 고정 (나중에 DB에서 가져오도록 교체 예정)
    constexpr float TempBaseDamage = 10.f;

    // ✅ FIX: "공통 베이스 데미지"는 WeaponGameplayAbility의 공통 함수로 적용
    bAnyApplied |= ApplyBaseDamageToTargetActor(TargetActor, TempBaseDamage, /*Level*/ 1.f, /*Chance*/ 1.f);

   
    // ✅ FIX: 프로파일 OnHitTargetEffects는 "특수효과 전용" (출혈/화염/빙결/스턴 등)
    //        BP에서 GE만 추가해도 그대로 동작하게 유지
    for (const FOnHitGameplayEffectSpec& Spec : CachedProfile.OnHitTargetEffects)
    {
        if (!Spec.Effect)
        {
            continue;
        }

        // 실수로 OnHit에 기본 데미지 GE를 넣었다면 중복 데미지 방지
        if (BaseDamageEffectClass && Spec.Effect && Spec.Effect->IsChildOf(BaseDamageEffectClass))
        {
            continue;
        }

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

void UWeaponMeleeAttackAbilityBase::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UWeaponMeleeAttackAbilityBase::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UWeaponMeleeAttackAbilityBase::OnMontageInterrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
}

void UWeaponMeleeAttackAbilityBase::OnMontageBlendOut()
{
    // 블렌드아웃에서 종료시키고 싶으면 여기서 EndAbility 호출해도 됨.
    // 지금은 Completed/Interrupted가 처리하므로 비워둬도 OK.
}