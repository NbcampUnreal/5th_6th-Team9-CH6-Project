#include "Weapons/GameAbility/MeleeAbility/WeaponMeleeAttackAbilityBase.h"

#include "Weapons/MeleeWeaponBase/MeleeWeaponBase.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

static void DrawHitBoxDebug(AMeleeWeaponBase* Weapon, const FColor& Color, float LifeTime = 0.25f)
{
    if (!Weapon) return;

    UBoxComponent* HitBox = Weapon->GetHitBox();
    if (!HitBox) return;

    UWorld* World = Weapon->GetWorld();
    if (!World) return;

    DrawDebugBox(
        World,
        HitBox->GetComponentLocation(),
        HitBox->GetScaledBoxExtent(),
        HitBox->GetComponentQuat(),
        Color,
        false,      // PersistentLines
        LifeTime,   // LifeTime
        0,
        2.0f        // Thickness
    );
}

UWeaponMeleeAttackAbilityBase::UWeaponMeleeAttackAbilityBase()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    HitWindowOnEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Melee.Hitbox.On"), false);
    HitWindowOffEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Melee.Hitbox.Off"), false);
}

void UWeaponMeleeAttackAbilityBase::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    AMeleeWeaponBase* Weapon = GetWeaponFromSourceObject<AMeleeWeaponBase>();
    if (!Weapon)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }

    if (!CacheProfileFromWeapon(Weapon))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, true);
        return;
    }

    Weapon->SetHitBoxEnabled(false);
    BindHitBoxOverlap(Weapon);

    if (HitWindowOnEventTag.IsValid())
    {
        HitWindowOnTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, HitWindowOnEventTag, nullptr, false, true);
        if (HitWindowOnTask)
        {
            HitWindowOnTask->EventReceived.AddDynamic(this, &ThisClass::OnHitWindowOnEventReceived);
            HitWindowOnTask->ReadyForActivation();
        }
    }

    if (HitWindowOffEventTag.IsValid())
    {
        HitWindowOffTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, HitWindowOffEventTag, nullptr, false, true);
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
    bool bWasCancelled)
{
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
    if (!Weapon) return false;

    // ✅ (2단계) InputTag 추출은 이제 WeaponGameplayAbility(1단계) 공용 헬퍼를 사용
    const FGameplayTag InputTag = GetInputTagFromCurrentSpec();
    if (!InputTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[MeleeGA] Missing InputTag in AbilitySpec. Ability=%s"), *GetNameSafe(this));
        return false;
    }

    FWeaponAttackProfile Profile;
    if (!Weapon->GetAttackProfile(InputTag, Profile))
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

void UWeaponMeleeAttackAbilityBase::OnHitWindowOnEventReceived(FGameplayEventData Payload)
{
    AMeleeWeaponBase* Weapon = GetWeaponFromSourceObject<AMeleeWeaponBase>();
    if (!Weapon || !bHasCachedProfile) return;

    HitActors.Reset();
    Weapon->SetHitBoxEnabled(true);

    // 공격 시작 시 히트박스 모양 디버그 표시
    DrawHitBoxDebug(Weapon, FColor::Green, 0.25f);
}

void UWeaponMeleeAttackAbilityBase::OnHitWindowOffEventReceived(FGameplayEventData Payload)
{
    AMeleeWeaponBase* Weapon = GetWeaponFromSourceObject<AMeleeWeaponBase>();
    if (!Weapon) return;

    // 공격 끝 시 히트박스 모양 디버그 표시
    DrawHitBoxDebug(Weapon, FColor::Green, 0.25f);

    Weapon->SetHitBoxEnabled(false);
}

void UWeaponMeleeAttackAbilityBase::OnHitBoxBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!bHasCachedProfile || !OtherActor) return;

    AActor* Avatar = GetAvatarActorFromActorInfo();
    AMeleeWeaponBase* Weapon = GetWeaponFromSourceObject<AMeleeWeaponBase>();
    if (!Avatar || !Weapon) return;

    if (OtherActor == Avatar || OtherActor == Weapon) return;

    if (CachedProfile.bHitEachActorOnce && HitActors.Contains(OtherActor))
    {
        return;
    }

    if (!ApplyOnHitEffects(OtherActor))
    {
        return;
    }

    const FVector SpawnLoc =
        OtherComp ? OtherComp->GetComponentLocation() : OtherActor->GetActorLocation();

    SpawnWeaponHitImpactFXAtLocation(SpawnLoc, FVector::UpVector);


    HitActors.Add(OtherActor);

    if (CachedProfile.bHitFirstTargetOnly)
    {
        Weapon->SetHitBoxEnabled(false);
    }
}

bool UWeaponMeleeAttackAbilityBase::ApplyOnHitEffects(AActor* TargetActor)
{
    if (!TargetActor) return false;

    bool bAnyApplied = false;

    // 주 데미지(WeaponDamage(DT) * DamageMultiplier)
    bAnyApplied |= ApplyWeaponDamageToTargetActor(TargetActor, CachedProfile.DamageMultiplier, 1.f, 1.f, 0.f);

    // 부가효과: Data.EnemyDamage 중복 방지
    const FGameplayTag DamageTag = GetDataDamageTag();

    for (const FOnHitGameplayEffectSpec& Spec : CachedProfile.OnHitTargetEffects)
    {
        if (!Spec.Effect) continue;

        TMap<FGameplayTag, float> Mags = Spec.SetByCallerMagnitudes;
        if (DamageTag.IsValid())
        {
            Mags.Remove(DamageTag);
        }

        bAnyApplied |= ApplyEffectToTargetActor(TargetActor, Spec.Effect, Spec.Level, Mags, Spec.Chance);
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
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}