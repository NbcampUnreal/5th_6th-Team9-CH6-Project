// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SBNameInputDialogWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNameInputOk, FString, WorldName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNameInputCancel);

UCLASS()
class STILLBOUND_API USBNameInputDialogWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable) FOnNameInputOk OnOk;
    UPROPERTY(BlueprintAssignable) FOnNameInputCancel OnCancel;

    UFUNCTION(BlueprintCallable) void SetTitle(const FText& InTitle);

protected:
    UPROPERTY(meta = (BindWidget)) UEditableTextBox* ETB_WorldName;
    UPROPERTY(meta = (BindWidget)) UButton* BTN_OK;
    UPROPERTY(meta = (BindWidget)) UButton* BTN_Cancel;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TXT_Title;

    virtual void NativeConstruct() override;

    UFUNCTION() void HandleOk();
    UFUNCTION() void HandleCancel();
};