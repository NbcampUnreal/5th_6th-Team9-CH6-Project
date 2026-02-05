// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/DialogueWidget.h"
#include "DialogueComponent.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "DialogueOptionButton.h"
#include "Blueprint/WidgetTree.h"
#include "NPCCharacter.h"

void UDialogueWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(this->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

        PC->SetInputMode(InputMode);

        //마우스 커서 보이게
        PC->SetShowMouseCursor(true);
    }
}

void UDialogueWidget::ShowDialogue(const FDialogueRow& DialogueData)
{
    CurrentDialogueData = DialogueData;

    // 블루프린트에서 UI 업데이트 처리
    // (텍스트 블록, 버튼 등)
    if (TXT_DialogueContent)
    {
        TXT_DialogueContent->SetText(DialogueData.DialogueText);
        UE_LOG(LogTemp, Log, TEXT("위젯 본문 세팅 완료: %s"), *DialogueData.DialogueText.ToString());
    }

    if (TXT_NPCName)
    {
        TXT_NPCName->SetText(DialogueData.NPCName);
    }
    if (TXT_DialogueContent)
    {
        TXT_DialogueContent->SetText(DialogueData.DialogueText);
    }
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
        VB_OptionList->ClearChildren();
        for (int32 i = 0; i < DialogueData.Options.Num(); i++)
        {
            if (OptionButtonClass)
            {
                UDialogueOptionButton* NewButton = CreateWidget<UDialogueOptionButton>(this, OptionButtonClass);

                if (NewButton)
                {
                    NewButton->InitializeOption(i, DialogueData.Options[i].OptionText);
                    NewButton->OnOptionClicked.AddDynamic(this, &UDialogueWidget::HandleOptionSelected);
                    VB_OptionList->AddChild(NewButton);
                }
            }
        }
    }
}

void UDialogueWidget::HandleOptionSelected(int32 OptionIndex)
{

    UE_LOG(LogTemp, Log, TEXT("옵션 선택됨: %d"), OptionIndex);

    if (OnOptionClicked.IsBound())
    {
        OnOptionClicked.Broadcast(OptionIndex);
    }
}

void UDialogueWidget::CloseDialogue()
{
    //입력모드복원
    RemoveFromParent();

    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }
}


void UDialogueWidget::SetDialogueComponent(UDialogueComponent* Component)
{
    DialogueComponent = Component;
}

void UDialogueWidget::UpdateContent(const FText& Name, const FText& Content, const TArray<FText>& Options)
{
    if (TXT_NPCName) TXT_NPCName->SetText(Name);

    if (TXT_DialogueContent) TXT_DialogueContent->SetText(Content);

    if (VB_OptionList)
    {
        VB_OptionList->ClearChildren();
    }

    if (OptionButtonClass && VB_OptionList)
    {
        for (int32 i = 0; i < Options.Num(); i++)
        {
            UDialogueOptionButton* NewButton = CreateWidget<UDialogueOptionButton>(this, OptionButtonClass);

            if (NewButton)
            {
                //데이터 세팅
                NewButton->InitializeOption(i, Options[i]);

                //델리게이트 연결
                NewButton->OnOptionClicked.AddDynamic(this, &UDialogueWidget::HandleOptionSelected);

                //vertical box에 붙이기
                VB_OptionList->AddChild(NewButton);
            }
        }
    }

}
