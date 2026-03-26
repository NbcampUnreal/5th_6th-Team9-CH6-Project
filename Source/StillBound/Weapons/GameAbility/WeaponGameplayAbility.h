#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "Engine/EngineTypes.h"  
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
    // 공격 시작 시 공통 상태 태그 부여, 종료 시 제거
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled
    ) override;

    // 공격 중 에임 방향 몸회전용 상태 태그 관리
    void AddFaceAimStateTag();
    void RemoveFaceAimStateTag();

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

    /** 기본 데미지(공통) 적용 헬퍼 */
    bool ApplyBaseDamageToTargetActor(
        AActor* TargetActor,
        float DamageValue,
        float Level = 1.0f,
        float Chance = 1.0f
    ) const;

protected:
    /** 기본 데미지에 사용할 GE (기본값: UGE_WeaponDamage_Instant) */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Damage")
    TSubclassOf<UGameplayEffect> BaseDamageEffectClass;

    /** GE 적용 디버그 로그 토글 */
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Debug")
    bool bDebugGE = true;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Debug")
    bool bDebugHitFX = true;

    //  공격 중 카메라/에임 방향 몸회전용 태그 사용 여부
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|State")
    bool bUseFaceAimStateTag = true;

    //  공격 중 캐릭터가 감지할 상태 태그
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|State")
    FGameplayTag FaceAimStateTag;

    //  상태 태그 디버그 로그
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|GA|Debug")
    bool bDebugStateTag = true;

    //  이번 활성화에서 직접 태그를 넣었는지 기억
    UPROPERTY(Transient)
    bool bAddedFaceAimStateTagThisActivation = false;

    // ✅ SourceObject(Weapon)에서 데미지를 가져온다.
    // WeaponDamage가 0이면 FallbackDamage(예: BaseDamage)를 사용.
    UFUNCTION(BlueprintPure, Category = "Weapon|Damage")
    float GetDamageFromWeaponOrFallback(float FallbackDamage = 0.f) const;

    // ✅ "무기 데미지"를 기본 데미지 GE(Data.EnemyDamage)로 적용한다.
    // DamageMultiplier로 공격 유형별 배율(예: Light=1.0, Heavy=1.6)도 지원.
    UFUNCTION(BlueprintCallable, Category = "Weapon|Damage")
    bool ApplyWeaponDamageToTargetActor(
        AActor* TargetActor,
        float DamageMultiplier = 1.f,
        float Level = 1.f,
        float Chance = 1.f,
        float FallbackDamage = 0.f
    ) const;

    // 수정: 히트 결과 기준으로 공통 히트 FX 스폰
    bool SpawnWeaponHitImpactFXFromHitResult(const FHitResult& HitResult) const;

    // 수정: 위치/노멀만 있을 때도 공통 히트 FX 스폰 가능
    bool SpawnWeaponHitImpactFXAtLocation(
        const FVector& SpawnLocation,
        const FVector& ImpactNormal = FVector::UpVector
    ) const;

protected:
    /** 현재 실행 중인 AbilitySpec을 찾는다 (CurrentSpecHandle 기반) */
    const FGameplayAbilitySpec* FindCurrentAbilitySpec() const;

    /** AbilitySpec.DynamicAbilityTags에서 InputTag.* (첫 번째) 반환. 없으면 Invalid */
    UFUNCTION(BlueprintPure, Category = "Weapon|GA")
    FGameplayTag GetInputTagFromCurrentSpec() const;

    /** 위 함수의 bool 버전 */
    bool TryGetInputTagFromCurrentSpec(FGameplayTag& OutInputTag) const;

public:
    /** SetByCaller에 사용할 데미지 태그 */
    static FGameplayTag GetDataDamageTag();
};