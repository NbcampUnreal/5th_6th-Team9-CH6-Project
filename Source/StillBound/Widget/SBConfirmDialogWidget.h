// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SBConfirmDialogWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfirmYes);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfirmNo);

UCLASS()
class STILLBOUND_API USBConfirmDialogWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable) FOnConfirmYes OnYes;
    UPROPERTY(BlueprintAssignable) FOnConfirmNo  OnNo;

    UFUNCTION(BlueprintCallable)
    void SetMessage(const FText& InMessage);

protected:
    UPROPERTY(meta = (BindWidget)) UButton* BTN_Yes;
    UPROPERTY(meta = (BindWidget)) UButton* BTN_No;
    UPROPERTY(meta = (BindWidget)) UTextBlock* TXT_Message;

    virtual void NativeConstruct() override;

    UFUNCTION() void HandleYes();
    UFUNCTION() void HandleNo();
};