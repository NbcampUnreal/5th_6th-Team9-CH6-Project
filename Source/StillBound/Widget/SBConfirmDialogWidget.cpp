// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SBConfirmDialogWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void USBConfirmDialogWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (BTN_Yes) BTN_Yes->OnClicked.AddDynamic(this, &ThisClass::HandleYes);
    if (BTN_No)  BTN_No->OnClicked.AddDynamic(this, &ThisClass::HandleNo);
}

void USBConfirmDialogWidget::SetMessage(const FText& InMessage)
{
    if (TXT_Message) TXT_Message->SetText(InMessage);
}

void USBConfirmDialogWidget::HandleYes()
{
    OnYes.Broadcast();
    RemoveFromParent();
}

void USBConfirmDialogWidget::HandleNo()
{
    OnNo.Broadcast();
    RemoveFromParent();
}
