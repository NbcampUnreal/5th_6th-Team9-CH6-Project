// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/GameAbility/RangedAbility/WeaponRangedAttackAbilityBase.h"
#include "GA_Projectile.generated.h"

/**
 * 
 */
UCLASS()
class STILLBOUND_API UGA_Projectile : public UWeaponRangedAttackAbilityBase
{
	GENERATED_BODY()
	
public:
	UGA_Projectile();

protected:
	virtual void FireCurrentProfile(class ARangedWeaponBase* Weapon) override;

};
