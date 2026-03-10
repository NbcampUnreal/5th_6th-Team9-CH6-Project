#include "Weapons/WeaponBase.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Items/ItemBase.h"

AWeaponBase::AWeaponBase()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);
}

void AWeaponBase::Equip(AActor* NewOwner, UAbilitySystemComponent* InASC)
{
    if (bEquipped)
    {
        Unequip();
    }

    if (!NewOwner || !InASC)
    {
        return;
    }

    EquippedOwner = NewOwner;
    EquippedASC = InASC;

    SetOwner(NewOwner);

    GrantToASC(InASC);
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

    FGameplayTagContainer AbilityTagContainer; // ? 이름 변경 (Tags 금지)
    AbilityTagContainer.AddTag(InputTag);

    return EquippedASC->TryActivateAbilitiesByTag(AbilityTagContainer);
}

void AWeaponBase::ApplyWeaponTypeTag(UAbilitySystemComponent* ASC, bool bAdd) const
{
    if (!ASC || !WeaponTypeTag.IsValid())
    {
        return;
    }

    if (bAdd)
    {
        ASC->AddLooseGameplayTag(WeaponTypeTag);
    }
    else
    {
        ASC->RemoveLooseGameplayTag(WeaponTypeTag);
    }
}

void AWeaponBase::GrantToASC(UAbilitySystemComponent* ASC)
{
    if (!ASC)
    {
        return;
    }

    GrantedHandles.Reset();

    ApplyWeaponTypeTag(ASC, true);

    // Abilities
    for (const FWeaponAbilityGrant& Grant : GrantedAbilities)
    {
        if (!Grant.Ability)
        {
            continue;
        }

        FGameplayAbilitySpec Spec(Grant.Ability, Grant.AbilityLevel);
        Spec.SourceObject = this; // GA에서 무기 접근용

        if (Grant.InputTag.IsValid())
        {
            Spec.DynamicAbilityTags.AddTag(Grant.InputTag);
        }

        const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
        if (Handle.IsValid())
        {
            GrantedHandles.AbilityHandles.Add(Handle);
        }
    }

    // Effects (Self)
    for (const FWeaponEffectGrant& Grant : GrantedEffects)
    {
        if (!Grant.Effect)
        {
            continue;
        }

        FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
        Ctx.AddSourceObject(this);

        const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Grant.Effect, Grant.EffectLevel, Ctx);
        if (!SpecHandle.IsValid())
        {
            continue;
        }

        const FActiveGameplayEffectHandle ActiveHandle =
            ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

        if (ActiveHandle.IsValid())
        {
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

    ApplyWeaponTypeTag(ASC, false);

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
    if (!Item)
    {
        return;
    }

    // 네 ItemBase 구조에서 여기 경로만 맞추면 됨
    WeaponDamage = Item->ItemStatistics.DamageValue;
}