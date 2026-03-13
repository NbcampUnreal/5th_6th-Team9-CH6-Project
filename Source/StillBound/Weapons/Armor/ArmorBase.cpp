// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Armor/ArmorBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Items/ItemBase.h"

AArmorBase::AArmorBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ArmorDefenseSetByCallerTag =
		FGameplayTag::RequestGameplayTag(TEXT("Data.ArmorDefense"), false);
}

void AArmorBase::InitFromItem(const UItemBase* Item)
{
	if (!Item)
	{
		return;
	}

	ArmorRating = Item->ItemStatistics.ArmorRating;
}

void AArmorBase::GrantToASC(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}

	GrantedHandles.Reset();

	// 부모의 공통 태그 처리 재사용
	ApplyWeaponTypeTag(ASC, true);

	// 필요하면 방어구도 Ability 줄 수 있게 유지
	for (const FWeaponAbilityGrant& Grant : GrantedAbilities)
	{
		if (!Grant.Ability)
		{
			continue;
		}

		FGameplayAbilitySpec Spec(Grant.Ability, Grant.AbilityLevel);
		Spec.SourceObject = this;

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

	// 공용 GE 하나에 ArmorRating을 주입
	for (const FWeaponEffectGrant& Grant : GrantedEffects)
	{
		if (!Grant.Effect)
		{
			continue;
		}

		FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
		Ctx.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle =
			ASC->MakeOutgoingSpec(Grant.Effect, Grant.EffectLevel, Ctx);

		if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
		{
			continue;
		}

		if (ArmorDefenseSetByCallerTag.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(
				ArmorDefenseSetByCallerTag,
				ArmorRating
			);
		}

		const FActiveGameplayEffectHandle ActiveHandle =
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		if (ActiveHandle.IsValid())
		{
			GrantedHandles.EffectHandles.Add(ActiveHandle);
		}
	}
}

void AArmorBase::RevokeFromASC(UAbilitySystemComponent* ASC)
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