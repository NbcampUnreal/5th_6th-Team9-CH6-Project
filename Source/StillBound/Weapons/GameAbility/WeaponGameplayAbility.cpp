// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GameAbility/WeaponGameplayAbility.h"

#include "Weapons/WeaponBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"

UWeaponGameplayAbility::UWeaponGameplayAbility()
{
    // 싱글플레이 기준: LocalOnly로 두면 예측/서버 이슈 없이 깔끔함
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;

    // 보통 무기 공격은 인스턴스가 필요 (상태/타이머/히트목록 유지)
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

AWeaponBase* UWeaponGameplayAbility::GetWeaponFromSourceObject() const
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC)
    {
        return nullptr;
    }

    const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(CurrentSpecHandle);
    if (!Spec)
    {
        return nullptr;
    }

    UObject* SourceObj = Spec->SourceObject.Get();
    return Cast<AWeaponBase>(SourceObj);
}

UAbilitySystemComponent* UWeaponGameplayAbility::GetTargetASC(AActor* TargetActor) const
{
    if (!TargetActor)
    {
        return nullptr;
    }
    return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
}

bool UWeaponGameplayAbility::ApplyEffectToTargetActor(
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

    UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    UAbilitySystemComponent* TargetASC = GetTargetASC(TargetActor);
    if (!SourceASC || !TargetASC)
    {
        return false;
    }

    AActor* Avatar = GetAvatarActorFromActorInfo();
    AWeaponBase* Weapon = GetWeaponFromSourceObject();

    FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
    if (Avatar)
    {
        Ctx.AddInstigator(Avatar, Weapon ? Cast<AActor>(Weapon) : Avatar);
    }
    if (Weapon)
    {
        Ctx.AddSourceObject(Weapon);
    }

    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, Level, Ctx);
    if (!SpecHandle.IsValid())
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