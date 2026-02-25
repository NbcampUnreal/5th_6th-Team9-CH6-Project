// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBase.h"

#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffect.h"
#include "Components/SkeletalMeshComponent.h"
#include "Items/ItemBase.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);
}

void AWeaponBase::Equip(AActor* NewOwner, UAbilitySystemComponent* InASC)
{

    UE_LOG(LogTemp, Warning, TEXT("[DBG] AWeaponBase::Equip %s"), *GetName());

    if (bEquipped)
    {
        Unequip();
    }

    EquippedOwner = NewOwner;
    EquippedASC = InASC;

    if (!EquippedOwner.IsValid() || !EquippedASC.IsValid())
    {
        EquippedOwner = nullptr;
        EquippedASC = nullptr;
        return;
    }

    SetOwner(EquippedOwner.Get());

    GrantToASC(EquippedASC.Get());
    bEquipped = true;
}

void AWeaponBase::Unequip()
{
    if (!bEquipped)
    {
        return;
    }

    if (EquippedASC.IsValid())
    {
        RevokeFromASC(EquippedASC.Get());
    }

    bEquipped = false;
    EquippedASC = nullptr;
    EquippedOwner = nullptr;
    SetOwner(nullptr);
}

bool AWeaponBase::ActivateByInputTag(FGameplayTag InputTag)
{
    if (!bEquipped || !EquippedASC.IsValid() || !InputTag.IsValid())
    {
        return false;
    }

    const FGameplayAbilitySpecHandle* Found = GrantedHandles.InputToAbilityHandle.Find(InputTag);
    if (!Found || !Found->IsValid())
    {
        return false;
    }

    return EquippedASC->TryActivateAbility(*Found);
}

void AWeaponBase::GrantToASC(UAbilitySystemComponent* ASC)
{
    if (!ASC)
    {
        UE_LOG(LogTemp, Error, TEXT("[DBG] GrantToASC ASC is NULL"));
        return;
    }

    GrantedHandles.Reset();
    //상태태그 추가
  // 무기 BP의 WeaponTypeTag를 ASC에 퍼블리시
    if (WeaponTypeTag.IsValid())
    {
        ASC->AddLooseGameplayTag(WeaponTypeTag);
    }

   

    UE_LOG(LogTemp, Warning, TEXT("[DBG] GrantToASC: Weapon=%s AbilitiesToGrant=%d OwnerHasAuthority=%d"),
        *GetName(), GrantedAbilities.Num(),
        GetOwner() ? (int32)GetOwner()->HasAuthority() : -1);
    // Abilities
    for (const FWeaponAbilityGrant& Grant : GrantedAbilities)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DBG] Grant Entry: Ability=%s InputTag=%s Valid=%d Level=%d"),
            *GetNameSafe(Grant.Ability),
            *Grant.InputTag.ToString(),
            Grant.InputTag.IsValid(),
            Grant.AbilityLevel);

        if (!Grant.Ability)
        {
            UE_LOG(LogTemp, Warning, TEXT("[DBG] -> Skip (Ability is null)"));
            continue;
        }

        FGameplayAbilitySpec Spec(Grant.Ability, Grant.AbilityLevel);
        Spec.SourceObject = this; // GA에서 무기 데이터를 읽기 위해 필수

        if (Grant.InputTag.IsValid())
        {
            Spec.DynamicAbilityTags.AddTag(Grant.InputTag);
        }

        const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);

        UE_LOG(LogTemp, Warning, TEXT("[DBG] GiveAbility: HandleValid=%d"),
            Handle.IsValid());

        GrantedHandles.AbilityHandles.Add(Handle);

        if (Grant.InputTag.IsValid())
        {
            GrantedHandles.InputToAbilityHandle.Add(Grant.InputTag, Handle);
        }
    }

    // Effects (필요하면 사용, 몽둥이 예시는 없어도 됨)
    for (const FWeaponEffectGrant& Grant : GrantedEffects)
    {
        if (!Grant.Effect)
        {
            continue;
        }

        FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
        Ctx.AddSourceObject(this);

        const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Grant.Effect, Grant.EffectLevel, Ctx);
        if (SpecHandle.IsValid())
        {
            const FActiveGameplayEffectHandle ActiveHandle =
                ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

            GrantedHandles.EffectHandles.Add(ActiveHandle);
        }
    }
}

void AWeaponBase::RevokeFromASC(UAbilitySystemComponent* ASC)
{
    if (!ASC)
    {
        GrantedHandles.Reset();
        return;
    }

    //상태태그 추가
  // Equip 때 올린 WeaponTypeTag 회수
    if (WeaponTypeTag.IsValid())
    {
        ASC->RemoveLooseGameplayTag(WeaponTypeTag);
    }

    for (const FActiveGameplayEffectHandle& Handle : GrantedHandles.EffectHandles)
    {
        if (Handle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(Handle);
        }
    }

    for (const FGameplayAbilitySpecHandle& Handle : GrantedHandles.AbilityHandles)
    {
        if (Handle.IsValid())
        {
            ASC->ClearAbility(Handle);
        }
    }

    GrantedHandles.Reset();
}


void AWeaponBase::InitFromItem(const UItemBase* Item)
{
    if (!Item) return;

    //  마스터 아이템 테이블의 DamageValue를 사용
    WeaponDamage = Item->ItemStatistics.DamageValue;

   
}


// Called when the game starts or when spawned
//void AWeaponBase::BeginPlay()
//{
//	Super::BeginPlay();
//}
//
//// Called every frame
//void AWeaponBase::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

