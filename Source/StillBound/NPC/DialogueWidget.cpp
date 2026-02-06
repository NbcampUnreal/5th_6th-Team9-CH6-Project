// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/DialogueWidget.h"
#include "NPC/DialogueComponent.h"
#include "NPC/DialogueOptionButton.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"

void UDialogueWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(this->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(true);
    }
}

void UDialogueWidget::ShowDialogue(const FDialogueRow& DialogueData)
{
    CurrentDialogueData = DialogueData;

    if (TXT_NPCName) TXT_NPCName->SetText(DialogueData.NPCName);
    if (TXT_DialogueContent) TXT_DialogueContent->SetText(DialogueData.DialogueText);

    if (IMG_Profile && DialogueData.NPCPortrait)
    {
        IMG_Profile->SetBrushFromTexture(DialogueData.NPCPortrait);
        IMG_Profile->SetVisibility(ESlateVisibility::Visible);
    }
    else if (IMG_Profile)
    {
        IMG_Profile->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (VB_OptionList)
    {
        for (UDialogueOptionButton* Btn : OptionButtonPool)
        {
            if (Btn) Btn->SetVisibility(ESlateVisibility::Collapsed);
        }

        for (int32 i = 0; i < DialogueData.Options.Num(); i++)
        {
            UDialogueOptionButton* Button = nullptr;

            if (OptionButtonPool.IsValidIndex(i) && OptionButtonPool[i])
            {
                Button = OptionButtonPool[i];
            }
            else if(OptionButtonClass)
            {
                Button = CreateWidget<UDialogueOptionButton>(this, OptionButtonClass);
                if (Button)
                {
                    Button->OnOptionClicked.AddDynamic(this, &UDialogueWidget::HandleOptionSelected);

                    VB_OptionList->AddChild(Button);
                    OptionButtonPool.Add(Button);
                }
            }
            if (Button)
            {
                Button->InitializeOption(i, DialogueData.Options[i].OptionText);

                Button->SetVisibility(ESlateVisibility::Visible);
            }
        }
    }
}

void UDialogueWidget::HandleOptionSelected(int32 OptionIndex)
{

    UE_LOG(LogTemp, Log, TEXT("¿É¼Ç ¼±ÅÃµÊ: %d"), OptionIndex);

    if (OnOptionClicked.IsBound())
    {
        OnOptionClicked.Broadcast(OptionIndex);
    }
}

void UDialogueWidget::CloseDialogue()
{

    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
    RemoveFromParent();

}

void UDialogueWidget::SetDialogueComponent(UDialogueComponent* Component)
{
    DialogueComponent = Component;
}
