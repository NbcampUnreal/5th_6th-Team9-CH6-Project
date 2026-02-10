// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/DialogueOptionButton.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UDialogueOptionButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Option)
	{
		// 1. 기존 연결이 있다면 끊어버리기 (안전장치)
		Btn_Option->OnClicked.RemoveDynamic(this, &UDialogueOptionButton::OnBtnClicked);

		// 2. 그 다음 깨끗하게 연결하기
		Btn_Option->OnClicked.AddDynamic(this, &UDialogueOptionButton::OnBtnClicked);
	}
}

void UDialogueOptionButton::InitializeOption(int32 Index, FText Content)
{
	MyOptionIndex = Index;

	if (TXT_OptionContent)
	{
		TXT_OptionContent->SetText(Content);
	}
}

void UDialogueOptionButton::OnBtnClicked()
{
	if (OnOptionClicked.IsBound())
	{
		OnOptionClicked.Broadcast(MyOptionIndex);
	}
}
