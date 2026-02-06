// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SBWorldRowWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Data/SBWorldSlotData.h"

void USBWorldRowWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    USBWorldSlotData* Data = Cast<USBWorldSlotData>(ListItemObject);
    if (!Data) return;

    if (TXT_WorldName)  TXT_WorldName->SetText(FText::FromString(Data->WorldName));
    if (TXT_Day)        TXT_Day->SetText(FText::AsNumber(Data->Day));
    if (TXT_PlayerName) TXT_PlayerName->SetText(FText::FromString(Data->PlayerName));
    if (TXT_Level)      TXT_Level->SetText(FText::AsNumber(Data->PlayerLevel));

    if (TXT_LastPlayed)
    {
        TXT_LastPlayed->SetText(FText::FromString(Data->LastPlayed.ToString()));
    }
}

void USBWorldRowWidget::NativeOnItemSelectionChanged(bool bIsSelected)
{
    if (!BD_Background) return;

    BD_Background->SetBrushColor(bIsSelected ? FLinearColor(0.0f, 0.55f, 1.0f, 0.25f)
        : FLinearColor(0.0f, 0.0f, 0.0f, 0.10f));
}