// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/DialogueComponent.h"
#include "Engine/DataTable.h"

// Sets default values for this component's properties
UDialogueComponent::UDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}
    

// Called when the game starts
void UDialogueComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

bool UDialogueComponent::StartDialogue(AActor* Interactor)
{
	if (!DialogueDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("DialogueDataTable is not set!"));
		return false;
	}
	if (bIsDialogueActive)
	{
		UE_LOG(LogTemp, Error, TEXT("Dialogue is already active!"));
		return false;
	}

	CurrentInteractor = Interactor;

	FDialogueRow* DialogueRow = LoadDialogueByID(StartDialogueID);
	if (!DialogueRow)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load dialogue ID: %d"), StartDialogueID);
		return false;
	}

	CurrentDialogue = *DialogueRow;
	bIsDialogueActive = true;

	//이벤트 발동
	OnDialogueStarted.Broadcast(CurrentDialogue);

	UE_LOG(LogTemp, Log, TEXT("Dialogue started: %s"), *CurrentDialogue.DialogueText.ToString());
	return true;

}

void UDialogueComponent::EndDialogue()
{
	if (!bIsDialogueActive) return;

	bIsDialogueActive = false;
	CurrentInteractor = nullptr;

	//이벤트발동
	OnDialogueEnded.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("Dialogue ended"));
}

bool UDialogueComponent::SelectOption(int32 OptionIndex)
{
	if (!bIsDialogueActive) return false;
	if (!CurrentDialogue.Options.IsValidIndex(OptionIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("InValid option index: %d"), OptionIndex);
		return false;
	}
	const FDialogueOption& SelectedOption = CurrentDialogue.Options[OptionIndex];

	UE_LOG(LogTemp, Log, TEXT("Option selected: %s"), *SelectedOption.OptionText.ToString());

	//다음 대화로 이동 or 종료
	if (SelectedOption.NextDialogueID == 0)
	{
		EndDialogue();
		return true;
	}
	else
	{
		GoToDialogue(SelectedOption.NextDialogueID);
		return true;
	}
}

void UDialogueComponent::GoToDialogue(int32 DialogueID)
{
	FDialogueRow* DialogueRow = LoadDialogueByID(DialogueID);
	if (!DialogueRow)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load dialogue ID: %d"), DialogueID);
		EndDialogue();
		return;
	}
	CurrentDialogue = *DialogueRow;

	//이벤트 발동
	OnDialogueUpdated.Broadcast(CurrentDialogue);

	UE_LOG(LogTemp, Log, TEXT("Dialogue updated: &s"), *CurrentDialogue.DialogueText.ToString());
}

FDialogueRow* UDialogueComponent::LoadDialogueByID(int32 DialogueID)
{
	if (!DialogueDataTable) return nullptr;

	//DataTable에서 Row 이름으로 찾기
	FString RowName = FString::FromInt(DialogueID);

	FDialogueRow* Row = DialogueDataTable->FindRow<FDialogueRow>(
		FName(*RowName),
		TEXT("DialogueComponent")
	);
	return Row;
}


