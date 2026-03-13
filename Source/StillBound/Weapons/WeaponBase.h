// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpec.h"
#include "WeaponBase.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;
class USceneComponent;
class UItemBase;

USTRUCT(BlueprintType)
struct FWeaponAbilityGrant
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
    TSubclassOf<UGameplayAbility> Ability = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
    int32 AbilityLevel = 1;

    /** 예: InputTag.Attack.Primary */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
    FGameplayTag InputTag;
};

USTRUCT(BlueprintType)
struct FWeaponEffectGrant
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
    TSubclassOf<UGameplayEffect> Effect = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
    float EffectLevel = 1.f;
};

USTRUCT()
struct FGrantedWeaponHandles
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FGameplayAbilitySpecHandle> AbilityHandles;

    UPROPERTY()
    TArray<FActiveGameplayEffectHandle> EffectHandles;

    void Reset()
    {
        AbilityHandles.Reset();
        EffectHandles.Reset();
    }
};

UCLASS(Abstract, Blueprintable)
class STILLBOUND_API AWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    AWeaponBase();

    /** 장착: ASC에 GA/GE 부여 + WeaponTypeTag 퍼블리시 */
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void Equip(AActor* NewOwner, UAbilitySystemComponent* InASC);

    /** 해제: 부여한 GA/GE 회수 + WeaponTypeTag 제거 */
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void Unequip();

    /** InputTag로 발동 (AbilitySpec.DynamicAbilityTags 기준) */
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    bool ActivateByInputTag(FGameplayTag InputTag);

    /** 무기 루트(필요하면 Attach/정렬은 캐릭터가 처리) */
    UFUNCTION(BlueprintPure, Category = "Weapon")
    USceneComponent* GetWeaponRoot() const { return Root; }

    /** 아이템(또는 DT)에서 스탯 주입 */
    UFUNCTION(BlueprintCallable, Category = "Weapon|Init")
    virtual void InitFromItem(const UItemBase* Item);

    UFUNCTION(BlueprintPure, Category = "Weapon|Stats")
    float GetWeaponDamage() const { return WeaponDamage; }

    UFUNCTION(BlueprintPure, Category = "Weapon|Tags")
    FGameplayTag GetWeaponTypeTag() const { return WeaponTypeTag; }

    UFUNCTION(BlueprintPure, Category = "Weapon|Equip")
    FName GetEquipSocketName() const { return EquipSocketName; }

protected:
    /** DT/아이템에서 주입 받은 공격력 캐시(SoT: DT) */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float WeaponDamage = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    TObjectPtr<USceneComponent> Root;

    /** 예: Weapon_R, Weapon_L / 비어있으면 캐릭터 기본 소켓 사용 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Equip")
    FName EquipSocketName = NAME_None;

    /** 예: Weapon.Melee.Club, Weapon.Ranged.Rifle */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Tags")
    FGameplayTag WeaponTypeTag;

    /** Equip 시 ASC에 GiveAbility */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
    TArray<FWeaponAbilityGrant> GrantedAbilities;

    /** Equip 시 ASC에 ApplyGE(Self) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
    TArray<FWeaponEffectGrant> GrantedEffects;

protected:
    virtual void GrantToASC(UAbilitySystemComponent* ASC);
    virtual void RevokeFromASC(UAbilitySystemComponent* ASC);

    void ApplyWeaponTypeTag(UAbilitySystemComponent* ASC, bool bAdd) const;

protected:
    UPROPERTY()
    bool bEquipped = false;

    UPROPERTY()
    TWeakObjectPtr<UAbilitySystemComponent> EquippedASC;

    UPROPERTY()
    TWeakObjectPtr<AActor> EquippedOwner;

    UPROPERTY()
    FGrantedWeaponHandles GrantedHandles;
};