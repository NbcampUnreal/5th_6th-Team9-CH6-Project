// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "SBWorldRowWidget.generated.h"

class UBorder;
class UTextBlock;
class USBWorldSlotData;

UCLASS()
class STILLBOUND_API USBWorldRowWidget : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

protected:
    UPROPERTY(meta = (BindWidget)) UBorder* BD_Background;

    UPROPERTY(meta = (BindWidget)) UTextBlock* TXT_WorldName;
    UPROPERTY(meta = (BindWidget)) UTextBlock* TXT_Day;
    UPROPERTY(meta = (BindWidget)) UTextBlock* TXT_PlayerName;
    UPROPERTY(meta = (BindWidget)) UTextBlock* TXT_Level;
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TXT_LastPlayed;

    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

    virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;
};