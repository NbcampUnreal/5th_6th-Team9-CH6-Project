// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "ArmorBase.generated.h"

class UItemBase;
class UAbilitySystemComponent;

UCLASS()
class STILLBOUND_API AArmorBase : public AWeaponBase
{
	GENERATED_BODY()
	
public:
	AArmorBase();

	virtual void InitFromItem(const UItemBase* Item) override;

	UFUNCTION(BlueprintPure, Category = "Armor|Stats")
	float GetArmorRating() const { return ArmorRating; }

protected:
	virtual void GrantToASC(UAbilitySystemComponent* ASC) override;
	virtual void RevokeFromASC(UAbilitySystemComponent* ASC) override;

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Armor|Stats")
	float ArmorRating = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armor|GAS")
	FGameplayTag ArmorDefenseSetByCallerTag;

};
