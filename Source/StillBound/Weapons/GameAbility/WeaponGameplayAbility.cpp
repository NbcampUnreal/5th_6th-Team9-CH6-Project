// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/GameAbility/WeaponGameplayAbility.h"

#include "Weapons/WeaponBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "GameplayTagsManager.h"

#include "Weapons/GameEffect/GE_WeaponDamage_Instant.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

// 현재 무기의 HitBox 모양 그대로 디버그 박스를 그린다.

UWeaponGameplayAbility::UWeaponGameplayAbility()
{
    // 싱글플레이 기준: LocalOnly
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;

    // 무기 공격은 상태(히트목록/타이머 등)가 필요하니 인스턴스 권장
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // 기본 데미지 GE (BP에서 교체 가능)
    BaseDamageEffectClass = UGE_WeaponDamage_Instant::StaticClass();
}

FGameplayTag UWeaponGameplayAbility::GetDataDamageTag()
{
    static const FGameplayTag Tag =
        FGameplayTag::RequestGameplayTag(TEXT("Data.EnemyDamage"), /*ErrorIfNotFound*/ false);

    ensureMsgf(Tag.IsValid(),
        TEXT("[GAS] GameplayTag 'Data.EnemyDamage' is not registered. Add it in Project Settings > GameplayTags or DefaultGameplayTags.ini"));

    return Tag;
}

AWeaponBase* UWeaponGameplayAbility::GetWeaponFromSourceObject() const
{
    const FGameplayAbilitySpec* Spec = FindCurrentAbilitySpec();
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
    if (!TargetActor || !EffectClass)
    {
        if (bDebugGE)
        {
            UE_LOG(LogTemp, Warning, TEXT("[GE] Invalid args Target=%s Effect=%s"),
                *GetNameSafe(TargetActor), *GetNameSafe(EffectClass));
        }
        return false;
    }

    // 확률 발동
    if (Chance < 1.0f)
    {
        const float Roll = FMath::FRand();
        if (Roll > Chance)
        {
            return false;
        }
    }

    UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    if (!SourceASC)
    {
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

    // 컨텍스트: instigator / source object 세팅
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
    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
    {
        if (bDebugGE)
        {
            UE_LOG(LogTemp, Warning, TEXT("[GE] MakeOutgoingSpec failed Effect=%s"), *GetNameSafe(EffectClass));
        }
        return false;
    }

    // SetByCaller 주입
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

bool UWeaponGameplayAbility::ApplyBaseDamageToTargetActor(
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

float UWeaponGameplayAbility::GetDamageFromWeaponOrFallback(float FallbackDamage) const
{
    const AWeaponBase* Weapon = GetWeaponFromSourceObject();
    const float WeaponDmg = Weapon ? Weapon->GetWeaponDamage() : 0.f;

    return (WeaponDmg > 0.f) ? WeaponDmg : FallbackDamage;
}

bool UWeaponGameplayAbility::ApplyWeaponDamageToTargetActor(
    AActor* TargetActor,
    float DamageMultiplier,
    float Level,
    float Chance,
    float FallbackDamage
) const
{
    if (!TargetActor) return false;

    const float Base = GetDamageFromWeaponOrFallback(FallbackDamage);
    const float Mult = FMath::Max(0.f, DamageMultiplier);
    const float FinalDamage = Base * Mult;

    if (FinalDamage <= 0.f) return false;

    if (bDebugGE)
    {
        const AWeaponBase* Weapon = GetWeaponFromSourceObject();
        UE_LOG(LogTemp, Log, TEXT("[GA] ApplyWeaponDamage Final=%.2f (Base=%.2f Mult=%.2f) Target=%s Weapon=%s"),
            FinalDamage, Base, Mult, *GetNameSafe(TargetActor), *GetNameSafe(Weapon));
    }

    return ApplyBaseDamageToTargetActor(TargetActor, FinalDamage, Level, Chance);
}

// //수정: 공통 히트 FX 스폰 (위치/노멀 직접 전달)
bool UWeaponGameplayAbility::SpawnWeaponHitImpactFXAtLocation(
    const FVector& SpawnLocation,
    const FVector& ImpactNormal
) const
{
    const AWeaponBase* Weapon = GetWeaponFromSourceObject();
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

    FVector FinalNormal = ImpactNormal;
    if (FinalNormal.IsNearlyZero())
    {
        FinalNormal = FVector::UpVector;
    }

    FRotator SpawnRotation = FRotator::ZeroRotator;
    if (FX.bUseImpactNormalRotation)
    {
        SpawnRotation = FinalNormal.Rotation();
    }

    // 로컬 오프셋을 현재 회전 기준으로 적용
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

// //수정: FHitResult 기반 공통 히트 FX 스폰
bool UWeaponGameplayAbility::SpawnWeaponHitImpactFXFromHitResult(const FHitResult& HitResult) const
{
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
        if (bDebugHitFX)
        {
            UE_LOG(LogTemp, Warning, TEXT("[HitFX] SpawnFromHitResult failed: no valid location Actor=%s"),
                *GetNameSafe(HitResult.GetActor()));
        }
        return false;
       
    }

    if (!HitResult.ImpactNormal.IsNearlyZero())
    {
        ImpactNormal = HitResult.ImpactNormal;
    }
    else if (!HitResult.Normal.IsNearlyZero())
    {
        ImpactNormal = HitResult.Normal;
    }

    return SpawnWeaponHitImpactFXAtLocation(SpawnLocation, ImpactNormal);
}

const FGameplayAbilitySpec* UWeaponGameplayAbility::FindCurrentAbilitySpec() const
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC)
    {
        return nullptr;
    }

    return ASC->FindAbilitySpecFromHandle(CurrentSpecHandle);
}

bool UWeaponGameplayAbility::TryGetInputTagFromCurrentSpec(FGameplayTag& OutInputTag) const
{
    OutInputTag = FGameplayTag();

    const FGameplayAbilitySpec* Spec = FindCurrentAbilitySpec();
    if (!Spec)
    {
        return false;
    }

    // 1) 정석: InputTag 루트 기반 매칭
    const FGameplayTag InputRoot = FGameplayTag::RequestGameplayTag(TEXT("InputTag"), /*ErrorIfNotFound*/ false);
    if (InputRoot.IsValid())
    {
        for (const FGameplayTag& Tag : Spec->DynamicAbilityTags)
        {
            if (Tag.IsValid() && Tag.MatchesTag(InputRoot))
            {
                OutInputTag = Tag;
                return true;
            }
        }
    }

    // 2) 안전장치: 루트 태그가 등록 안 된 경우 대비 prefix 매칭
    for (const FGameplayTag& Tag : Spec->DynamicAbilityTags)
    {
        if (Tag.IsValid() && Tag.ToString().StartsWith(TEXT("InputTag.")))
        {
            OutInputTag = Tag;
            return true;
        }
    }

    return false;
}

FGameplayTag UWeaponGameplayAbility::GetInputTagFromCurrentSpec() const
{
    FGameplayTag Tag;
    TryGetInputTagFromCurrentSpec(Tag);
    return Tag;
}