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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UGameplayAbility> Ability;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 AbilityLevel = 1;

    // 예: InputTag.Attack.Primary
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag InputTag;
};

USTRUCT(BlueprintType)
struct FWeaponEffectGrant
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UGameplayEffect> Effect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
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

    // 입력 태그 -> 부여된 AbilitySpecHandle 매핑 (무기가 직접 Activate하기 쉬움)
    UPROPERTY()
    TMap<FGameplayTag, FGameplayAbilitySpecHandle> InputToAbilityHandle;

    void Reset()
    {
        AbilityHandles.Reset();
        EffectHandles.Reset();
        InputToAbilityHandle.Reset();
    }
};


UCLASS(Abstract, Blueprintable)
class STILLBOUND_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void Equip(AActor* NewOwner, UAbilitySystemComponent* InASC);

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void Unequip();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    bool ActivateByInputTag(FGameplayTag InputTag);

     UFUNCTION(BlueprintCallable, Category = "Weapon")
    USceneComponent* GetWeaponRoot() const { return Root; }


    // 아이템 데이터(스탯 등) 주입
    UFUNCTION(BlueprintCallable, Category = "Weapon|Init")
    virtual void InitFromItem(const UItemBase* Item);

    UFUNCTION(BlueprintPure, Category = "Weapon|Stats")
    float GetWeaponDamage() const { return WeaponDamage; }

protected:
	// Called when the game starts or when spawned
    // 
      //  DT에서 주입받은 데미지 캐시
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Stats")
    float WeaponDamage = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Tags")
    FGameplayTag WeaponTypeTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
    TArray<FWeaponAbilityGrant> GrantedAbilities;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|GAS")
    TArray<FWeaponEffectGrant> GrantedEffects;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    FName AttachSocketName = NAME_None;

protected:
    virtual void GrantToASC(UAbilitySystemComponent* ASC);
    virtual void RevokeFromASC(UAbilitySystemComponent* ASC);

protected:
    UPROPERTY()
    bool bEquipped = false;

    UPROPERTY()
    TWeakObjectPtr<UAbilitySystemComponent> EquippedASC;

    UPROPERTY()
    TWeakObjectPtr<AActor> EquippedOwner;

    UPROPERTY()
    FGrantedWeaponHandles GrantedHandles;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;
protected:
    //virtual void BeginPlay() override;
};
