// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_StaminaBar.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_StaminaBar : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void SetStamina(float Current, float Max);

protected:

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* PB_Stamina;
};
