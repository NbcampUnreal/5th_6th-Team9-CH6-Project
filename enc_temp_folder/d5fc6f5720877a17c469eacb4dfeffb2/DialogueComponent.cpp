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
	UE_LOG(LogTemp, Error, TEXT("========================================"));
	UE_LOG(LogTemp, Error, TEXT("StartDialogue CALLED"));

	if (!MainMenuDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuDataTable is NULL!"));
		return false;
	}

	// 테이블 이름 확인
	FString MainTableName = MainMenuDataTable->GetName();
	UE_LOG(LogTemp, Error, TEXT("MainMenuDataTable Name: %s"), *MainTableName);

	// CurrentDataTable 설정
	CurrentDataTable = MainMenuDataTable;
	FString CurrentTableName = CurrentDataTable->GetName();
	UE_LOG(LogTemp, Error, TEXT("CurrentDataTable Name: %s"), *CurrentTableName);

	// 로드
	FDialogueRow* DialogueRow = LoadDialogueByID(StartDialogueID);
	if (!DialogueRow)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load dialogue ID: %d"), StartDialogueID);
		return false;
	}

	// 로드된 내용
	UE_LOG(LogTemp, Error, TEXT("Loaded DialogueText: %s"), *DialogueRow->DialogueText.ToString());
	UE_LOG(LogTemp, Error, TEXT("Options count: %d"), DialogueRow->Options.Num());

	for (int32 i = 0; i < DialogueRow->Options.Num(); i++)
	{
		UE_LOG(LogTemp, Error, TEXT("  Option %d: %s (Switch: %d)"),
			i,
			*DialogueRow->Options[i].OptionText.ToString(),
			(int32)DialogueRow->Options[i].SwitchToMenu);
	}

	CurrentDialogue = *DialogueRow;
	bIsDialogueActive = true;

	OnDialogueStarted.Broadcast(CurrentDialogue);

	UE_LOG(LogTemp, Error, TEXT("========================================"));
	return true;
}

void UDialogueComponent::EndDialogue()
{
	if (!bIsDialogueActive) return;

	bIsDialogueActive = false;
	CurrentInteractor = nullptr;
	CurrentMenuType = EMenuType::None;
	CurrentDataTable = nullptr;
	DialogueHistory.Empty();

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

	if (!CheckCondition(SelectedOption.RequiredCondition))  // FDialogueCondition을 전달
	{
		UE_LOG(LogTemp, Warning, TEXT("Option condition not met"));
		return false;
	}

	///메뉴 전환 확인
	if (SelectedOption.SwitchToMenu != EMenuType::None)
	{
		if (SelectedOption.SwitchToMenu == EMenuType::Exit)
		{
			EndDialogue();
			return true;
		}
		else
		{
			SwitchToMenu(SelectedOption.SwitchToMenu);
			return true;
		}

	}
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

bool UDialogueComponent::CheckCondition_Implementation(const FDialogueCondition& Condition)
{
	// 조건 없음
	if (Condition.ConditionType == EConditionType::None)
	{
		return true;
	}

	// TODO: 실제 조건 체크 로직 구현
	// 지금은 모든 조건 통과
	UE_LOG(LogTemp, Log, TEXT("CheckCondition: Type=%d, Value=%s, Amount=%d"),
		(int32)Condition.ConditionType, *Condition.ConditionValue, Condition.RequiredAmount);

	return true;
}

void UDialogueComponent::GoToDialogue(int32 DialogueID)
{
	if (!CurrentDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("No active DataTable!"));
		return;
	}

	// 이력 저장
	DialogueHistory.Add(CurrentDialogue.DialogueID);

	FDialogueRow* DialogueRow = LoadDialogueByID(DialogueID);
	if (!DialogueRow)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load dialogue ID: %d"), DialogueID);
		ReturnToMainMenu();
		return;
	}
	CurrentDialogue = *DialogueRow;

	//이벤트 발동
	OnDialogueUpdated.Broadcast(CurrentDialogue);

	UE_LOG(LogTemp, Log, TEXT("Dialogue updated: &s"), *CurrentDialogue.DialogueText.ToString());
}

void UDialogueComponent::SwitchToMenu(EMenuType MenuType)
{
	UDataTable* NewDataTable = GetDataTableForMenu(MenuType);

	if (!NewDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("DataTable not found for menu type: %d"), (int32)MenuType);
		ReturnToMainMenu();
		return;
	}

	// 메뉴 전환
	CurrentMenuType = MenuType;
	CurrentDataTable = NewDataTable;
	DialogueHistory.Empty();  // 메뉴 전환 시 이력 초기화

	// 해당 메뉴의 첫 대화 로드 (항상 ID 1부터 시작)
	FDialogueRow* DialogueRow = LoadDialogueByID(1);
	if (!DialogueRow)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load first dialogue in menu: %d"), (int32)MenuType);
		ReturnToMainMenu();
		return;
	}

	CurrentDialogue = *DialogueRow;

	OnMenuChanged.Broadcast(MenuType);
	OnDialogueUpdated.Broadcast(CurrentDialogue);

	UE_LOG(LogTemp, Log, TEXT("Switched to menu: %d"), (int32)MenuType);
}

void UDialogueComponent::ReturnToMainMenu()
{
	if (!MainMenuDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot return to main menu: MainMenuDataTable is null!"));
		EndDialogue();
		return;
	}

	CurrentMenuType = EMenuType::None;
	CurrentDataTable = MainMenuDataTable;
	DialogueHistory.Empty();

	FDialogueRow* DialogueRow = LoadDialogueByID(StartDialogueID);
	if (!DialogueRow)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load main menu!"));
		EndDialogue();
		return;
	}

	CurrentDialogue = *DialogueRow;

	OnDialogueUpdated.Broadcast(CurrentDialogue);

	UE_LOG(LogTemp, Log, TEXT("Returned to main menu"));
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

UDataTable* UDialogueComponent::GetDataTableForMenu(EMenuType MenuType)
{
	switch (MenuType)
	{
	case EMenuType::Dialogue:
		return DialogueDataTable;

	case EMenuType::Quest:
		return QuestDataTable;

	case EMenuType::Trade:
		return TradeDataTable;

	default:
		return nullptr;
	}
}