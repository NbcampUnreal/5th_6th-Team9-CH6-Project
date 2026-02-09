// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "WeaponGameplayAbility.generated.h"

class AWeaponBase;
class UGameplayEffect;
class UAbilitySystemComponent;


UCLASS(Abstract)
class STILLBOUND_API UWeaponGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UWeaponGameplayAbility();

protected:
    /** 현재 AbilitySpec의 SourceObject에서 무기(AWeaponBase)를 가져온다 */
    UFUNCTION(BlueprintCallable, Category = "Weapon|GA")
    AWeaponBase* GetWeaponFromSourceObject() const;

    template<typename T>
    T* GetWeaponFromSourceObject() const
    {
        return Cast<T>(GetWeaponFromSourceObject());
    }

    /** (공통 유틸) TargetActor의 ASC에 GE 적용 (SetByCaller 지원, Chance 지원) */
    bool ApplyEffectToTargetActor(
        AActor* TargetActor,
        TSubclassOf<UGameplayEffect> EffectClass,
        float Level,
        const TMap<FGameplayTag, float>& SetByCallerMagnitudes,
        float Chance = 1.0f
    ) const;

    /** ✅ FIX: "기본 데미지"를 모든 무기가 공통으로 적용할 수 있게 상위 헬퍼 추가 */
    bool ApplyBaseDamageToTargetActor(
        AActor* TargetActor,
        float DamageValue,
        float Level = 1.0f,
        float Chance = 1.0f
    ) const;

protected:
    /** ✅ FIX: 기본 데미지에 사용할 GE (기본값: UGE_WeaponDamage_Instant) */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Damage")
    TSubclassOf<UGameplayEffect> BaseDamageEffectClass;

    // [DEBUG ADD HERE] GE 적용 디버그 로그 토글
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Debug")
    bool bDebugGE = true;

public:
    static FGameplayTag GetDataDamageTag();
};
