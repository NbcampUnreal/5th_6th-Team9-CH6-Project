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

    if (TXT_MenuTitle)
    {
        if (DialogueComponent)
        {
            EMenuType CurrentMenu = DialogueComponent->GetCurrentMenuType();
            FText MenuTitle;

            switch (CurrentMenu)
            {
            case EMenuType::None:
                MenuTitle = FText::FromString(TEXT("MainMenu"));
                break;
            case EMenuType::Dialogue:
                MenuTitle = FText::FromString(TEXT("Talk"));
                break;
            case EMenuType::Quest:
                MenuTitle = FText::FromString(TEXT("Quest"));
                break;
            case EMenuType::Trade:
                MenuTitle = FText::FromString(TEXT("Trade"));
                break;
            default:
                MenuTitle = FText::FromString(TEXT(""));
            }

            TXT_MenuTitle->SetText(MenuTitle);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("DialogueWidget: DialogueComponent is NULL!"));
            TXT_MenuTitle->SetText(FText::FromString(TEXT("Talk")));
        }
    }

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

        int32 VisibleButtonIndex = 0;

        for (int32 i = 0; i < DialogueData.Options.Num(); i++)
        {
            const FDialogueOption& Option = DialogueData.Options[i];

            if (DialogueComponent)
            {
                if (!DialogueComponent->CheckCondition(Option.RequiredCondition))
                {
                    UE_LOG(LogTemp, Log, TEXT("DialogueWidget: Option %d condition not met, skipping"), i);
                    continue;
                }
            }

            UDialogueOptionButton* Button = nullptr;

            if (OptionButtonPool.IsValidIndex(VisibleButtonIndex) && OptionButtonPool[VisibleButtonIndex])
            {
                Button = OptionButtonPool[VisibleButtonIndex];
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
                VisibleButtonIndex++;
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
