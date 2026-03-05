// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UW_RoundProgressBar.generated.h"

/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_RoundProgressBar : public UUserWidget
{
	GENERATED_BODY()
	
public:

    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable)
    void SetPercent(float Percent);

protected:

    UPROPERTY(meta = (BindWidget))
    UImage* ProgressImage;

private:

    UMaterialInstanceDynamic* ProgressMaterial;
};
