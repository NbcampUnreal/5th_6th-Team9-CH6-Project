#include "Weapons/GameAbility/MeleeAbility/WeaponMeleeAttackAbilityBase.h"

#include "Weapons/MeleeWeaponBase/MeleeWeaponBase.h"
#include "AbilitySystemComponent.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"

#include "DrawDebugHelpers.h"

UWeaponMeleeAttackAbilityBase::UWeaponMeleeAttackAbilityBase()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    AttackTag = FGameplayTag::RequestGameplayTag(TEXT("Attack.Light"), false);

    HitWindowOnEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Melee.Hitbox.On"), false);
    HitWindowOffEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Melee.Hitbox.Off"), false);

    BaseDamage = 0.f;

    bDebugHitBoxBeginEnd = true;
    DebugBeginEndLifeTime = 2.0f;
    DebugBoxThickness = 2.0f;
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

    // 프로파일 기반 히트박스 설정 적용(BP에서 위치/회전은 직접 잡고, 여기서는 크기/채널 같은 값만)
    Weapon->ApplyOverlapConfig(CachedProfile.Overlap);

    // 공격 시작 시 히트박스는 항상 OFF
    Weapon->SetHitBoxEnabled(false);
    BindHitBoxOverlap(Weapon);

    // 히트 윈도우 이벤트 대기
    if (HitWindowOnEventTag.IsValid())
    {
        HitWindowOnTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
            this, HitWindowOnEventTag, nullptr, false, true);
        if (HitWindowOnTask)
        {
            HitWindowOnTask->EventReceived.AddDynamic(this, &ThisClass::OnHitWindowOnEventReceived);
            HitWindowOnTask->ReadyForActivation();
        }
    }

    if (HitWindowOffEventTag.IsValid())
    {
        HitWindowOffTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
            this, HitWindowOffEventTag, nullptr, false, true);
        if (HitWindowOffTask)
        {
            HitWindowOffTask->EventReceived.AddDynamic(this, &ThisClass::OnHitWindowOffEventReceived);
            HitWindowOffTask->ReadyForActivation();
        }
    }

    if (!CachedProfile.Montage)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }

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
}

void UWeaponMeleeAttackAbilityBase::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled
)
{
    // 종료 시 안전하게 정리
    AMeleeWeaponBase* Weapon = GetWeaponFromSourceObject<AMeleeWeaponBase>();
    if (Weapon)
    {
        Weapon->SetHitBoxEnabled(false);
        UnbindHitBoxOverlap(Weapon);
    }

    HitActors.Reset();
    bHasCachedProfile = false;

    MontageTask = nullptr;
    HitWindowOnTask = nullptr;
    HitWindowOffTask = nullptr;

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
    bHasCachedProfile = true;
    return true;
}

void UWeaponMeleeAttackAbilityBase::BindHitBoxOverlap(AMeleeWeaponBase* Weapon)
{
    if (!Weapon) return;

    UBoxComponent* HitBox = Weapon->GetHitBox();
    if (!HitBox) return;

    HitBox->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::OnHitBoxBeginOverlap);
    HitBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnHitBoxBeginOverlap);
}

void UWeaponMeleeAttackAbilityBase::UnbindHitBoxOverlap(AMeleeWeaponBase* Weapon)
{
    if (!Weapon) return;

    UBoxComponent* HitBox = Weapon->GetHitBox();
    if (!HitBox) return;

    HitBox->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::OnHitBoxBeginOverlap);
}

void UWeaponMeleeAttackAbilityBase::EnableHitWindow(AMeleeWeaponBase* Weapon)
{
    if (!Weapon) return;

    HitActors.Reset();
    Weapon->SetHitBoxEnabled(true);

    // ✅ 시작 시점에만 “히트박스 모양” 1회 출력
    if (bDebugHitBoxBeginEnd)
    {
        DrawHitBoxOnce(Weapon, FColor::Green, DebugBeginEndLifeTime);
    }
}

void UWeaponMeleeAttackAbilityBase::DisableHitWindow(AMeleeWeaponBase* Weapon)
{
    if (!Weapon) return;

    Weapon->SetHitBoxEnabled(false);

    // ✅ 끝 시점에만 “히트박스 모양” 1회 출력
    if (bDebugHitBoxBeginEnd)
    {
        DrawHitBoxOnce(Weapon, FColor::Red, DebugBeginEndLifeTime);
    }
}

void UWeaponMeleeAttackAbilityBase::DrawHitBoxOnce(AMeleeWeaponBase* Weapon, const FColor& Color, float LifeTime) const
{
    if (!Weapon) return;

    AActor* Avatar = GetAvatarActorFromActorInfo();
    UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
    if (!World) return;

    UBoxComponent* HitBox = Weapon->GetHitBox();
    if (!HitBox) return;

    const FTransform WT = HitBox->GetComponentTransform();
    const FVector Extent = HitBox->GetScaledBoxExtent(); // ✅ BP 스케일 반영

    DrawDebugBox(
        World,
        WT.GetLocation(),
        Extent,
        WT.GetRotation(),
        Color,
        false,
        LifeTime,
        0,
        DebugBoxThickness
    );
}

void UWeaponMeleeAttackAbilityBase::OnHitWindowOnEventReceived(FGameplayEventData Payload)
{
    AMeleeWeaponBase* Weapon = GetWeaponFromSourceObject<AMeleeWeaponBase>();
    if (!Weapon || !bHasCachedProfile) return;

    EnableHitWindow(Weapon);
}

void UWeaponMeleeAttackAbilityBase::OnHitWindowOffEventReceived(FGameplayEventData Payload)
{
    AMeleeWeaponBase* Weapon = GetWeaponFromSourceObject<AMeleeWeaponBase>();
    if (!Weapon || !bHasCachedProfile) return;

    DisableHitWindow(Weapon);
}

void UWeaponMeleeAttackAbilityBase::OnHitBoxBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    if (!bHasCachedProfile || !OtherActor)
    {
        return;
    }

    AActor* Avatar = GetAvatarActorFromActorInfo();
    AMeleeWeaponBase* Weapon = GetWeaponFromSourceObject<AMeleeWeaponBase>();
    if (!Avatar || !Weapon)
    {
        return;
    }

    if (OtherActor == Avatar || OtherActor == Weapon)
    {
        return;
    }

    if (CachedProfile.Overlap.bHitEachActorOnce &&
        HitActors.Contains(TWeakObjectPtr<AActor>(OtherActor)))
    {
        return;
    }

    const bool bApplied = ApplyOnHitEffects(OtherActor);
    if (!bApplied)
    {
        return;
    }

    HitActors.Add(OtherActor);

    if (CachedProfile.Overlap.bHitFirstTargetOnly)
    {
        DisableHitWindow(Weapon);
    }
}

bool UWeaponMeleeAttackAbilityBase::ApplyOnHitEffects(AActor* TargetActor)
{
    if (!TargetActor) return false;

    bool bAnyApplied = false;

    // ✅ DT → Weapon(WeaponDamage) → GA 로 연결 (방식 1)
    // 무기(SourceObject)에 주입된 데미지를 우선 사용하고,
    // 없으면(Base=0) 기존 BaseDamage를 fallback으로 사용.
    const AMeleeWeaponBase* Weapon = GetWeaponFromSourceObject<AMeleeWeaponBase>();
    const float WeaponDmg = Weapon ? Weapon->GetWeaponDamage() : 0.f;

    const float FinalDamage = (WeaponDmg > 0.f) ? WeaponDmg : BaseDamage;
    if (FinalDamage > 0.f)
    {
        bAnyApplied |= ApplyBaseDamageToTargetActor(TargetActor, FinalDamage, 1.f, 1.f);
    }

    // ✅ OnHitTargetEffects는 "부가효과" 용도로만 쓰는 걸 권장
    // (기존 데이터에 Data.EnemyDamage가 들어있으면 중복 데미지가 날 수 있어서 제거)
    const FGameplayTag DamageTag = GetDataDamageTag();

    for (const FOnHitGameplayEffectSpec& Spec : CachedProfile.OnHitTargetEffects)
    {
        if (!Spec.Effect) continue;

        TMap<FGameplayTag, float> Mags = Spec.SetByCallerMagnitudes;
        if (DamageTag.IsValid())
        {
            Mags.Remove(DamageTag); // 중복 데미지 방지
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
    // 필요 시 여기서 종료 처리 가능
}
