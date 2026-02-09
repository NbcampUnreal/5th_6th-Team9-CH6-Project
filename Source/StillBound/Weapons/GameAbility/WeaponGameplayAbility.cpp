// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/GameAbility/WeaponGameplayAbility.h"

#include "Weapons/WeaponBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "GameplayTagsManager.h"

#include "Weapons/GameEffect/GE_WeaponDamage_Instant.h"

UWeaponGameplayAbility::UWeaponGameplayAbility()
{
    // 싱글플레이 기준: LocalOnly로 두면 예측/서버 이슈 없이 깔끔함
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;

    // 보통 무기 공격은 인스턴스가 필요 (상태/타이머/히트목록 유지)
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    //  기본 데미지 GE 기본값 지정 (BP에서 다른 GE로 교체도 가능)
    BaseDamageEffectClass = UGE_WeaponDamage_Instant::StaticClass();
}

FGameplayTag UWeaponGameplayAbility::GetDataDamageTag() // 
{
    static const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("Data.EnemyDamage"), /*ErrorIfNotFound*/ false);
    ensureMsgf(Tag.IsValid(),
        TEXT("[GAS] GameplayTag 'Data.EnemyDamage' is not registered. Add it in Project Settings > GameplayTags or DefaultGameplayTags.ini"));
    return Tag;
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


bool UWeaponGameplayAbility::ApplyEffectToTargetActor(
    AActor* TargetActor,
    TSubclassOf<UGameplayEffect> EffectClass,
    float Level,
    const TMap<FGameplayTag, float>& SetByCallerMagnitudes,
    float Chance
) const
{
    // [DEBUG ADD HERE] 입력 검증 로그
    if (!TargetActor || !EffectClass)
    {
        if (bDebugGE)
        {
            UE_LOG(LogTemp, Warning, TEXT("[GE] Invalid args Target=%s Effect=%s"),
                *GetNameSafe(TargetActor), *GetNameSafe(EffectClass));
        }
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
    if (!SourceASC) {
        if (bDebugGE)
        {
            UE_LOG(LogTemp, Warning, TEXT("[GE] SourceASC is null"));
        }
        return false;
    }
    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!TargetASC)
    {
        if (bDebugGE)
        {
            UE_LOG(LogTemp, Warning, TEXT("[GE] TargetASC is null Target=%s"), *GetNameSafe(TargetActor));
        }
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
    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid()) // ✅ Data 유효성까지 체크
    {
        if (bDebugGE)
        {
            UE_LOG(LogTemp, Warning, TEXT("[GE] MakeOutgoingSpec failed Effect=%s"), *GetNameSafe(EffectClass));
        }
        return false;
    }


    for (const auto& KVP : SetByCallerMagnitudes)
    {
        if (KVP.Key.IsValid())
        {
            SpecHandle.Data->SetSetByCallerMagnitude(KVP.Key, KVP.Value);

            if (bDebugGE)
            {
                UE_LOG(LogTemp, Log, TEXT("[GE] SetByCaller %s=%.2f Effect=%s Target=%s"),
                    *KVP.Key.ToString(), KVP.Value, *GetNameSafe(EffectClass), *GetNameSafe(TargetActor));
            }
        }
    }

    SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
    return true;
}

bool UWeaponGameplayAbility::ApplyBaseDamageToTargetActor( // ✅ FIX: 구현 추가
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
        // 기본 데미지 GE가 지정되지 않았다면 적용 불가
        return false;
    }

    const FGameplayTag DamageTag = GetDataDamageTag();
    if (!DamageTag.IsValid())
    {
        return false;
    }

    TMap<FGameplayTag, float> Mags;
    Mags.Add(DamageTag, DamageValue); // SetByCaller(Data.Damage) 주입

    return ApplyEffectToTargetActor(
        TargetActor,
        BaseDamageEffectClass,
        Level,
        Mags,
        Chance
    );
}