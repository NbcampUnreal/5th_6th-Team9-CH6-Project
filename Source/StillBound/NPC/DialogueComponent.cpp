// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/DialogueComponent.h"
#include "NPC/NPCCharacter.h"
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

	if (MainMenuDataTable)
	{
		FString TableName = MainMenuDataTable->GetName();
		UE_LOG(LogTemp, Error, TEXT("MainMenuDataTable Name: %s"), *TableName);

		//Row Name 목록 출력
		TArray<FName> RowNames = MainMenuDataTable->GetRowNames();
		UE_LOG(LogTemp, Error, TEXT("Row Names in MainMenuDataTable:"));
		for (FName RowName : RowNames)
		{
			UE_LOG(LogTemp, Error, TEXT("  - '%s'"), *RowName.ToString());
		}
	}
	
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

	// 파일 전체 경로
	FString TablePath = MainMenuDataTable->GetPathName();
	UE_LOG(LogTemp, Error, TEXT("MainMenuDataTable FULL PATH: %s"), *TablePath);

	FString MainTableName = MainMenuDataTable->GetName();
	UE_LOG(LogTemp, Error, TEXT("MainMenuDataTable Name: %s"), *MainTableName);

	// 모든 Row 출력 (핵심!)
	TArray<FName> RowNames = MainMenuDataTable->GetRowNames();
	UE_LOG(LogTemp, Error, TEXT("========== TOTAL ROWS IN TABLE: %d =========="), RowNames.Num());

	for (FName RowName : RowNames)
	{
		FDialogueRow* Row = MainMenuDataTable->FindRow<FDialogueRow>(RowName, TEXT("Debug"));
		if (Row)
		{
			UE_LOG(LogTemp, Error, TEXT(">>> Row Name: '%s'"), *RowName.ToString());
			UE_LOG(LogTemp, Error, TEXT("    DialogueID: %d"), Row->DialogueID);
			UE_LOG(LogTemp, Error, TEXT("    DialogueText: %s"), *Row->DialogueText.ToString());
			UE_LOG(LogTemp, Error, TEXT("    Options Count: %d"), Row->Options.Num());

			for (int32 i = 0; i < Row->Options.Num(); i++)
			{
				UE_LOG(LogTemp, Error, TEXT("      Option %d: %s (Switch:%d, NextID:%d)"),
					i,
					*Row->Options[i].OptionText.ToString(),
					(int32)Row->Options[i].SwitchToMenu,
					Row->Options[i].NextDialogueID);
			}
		}
	}
	UE_LOG(LogTemp, Error, TEXT("=========================================="));

	// StartDialogueID 확인
	UE_LOG(LogTemp, Error, TEXT("StartDialogueID: %d"), StartDialogueID);

	if (bIsDialogueActive)
	{
		return false;
	}

	CurrentInteractor = Interactor;
	CurrentMenuType = EMenuType::None;
	CurrentDataTable = MainMenuDataTable;
	DialogueHistory.Empty();

	FDialogueRow* DialogueRow = LoadDialogueByID(StartDialogueID);
	if (!DialogueRow)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load dialogue ID: %d"), StartDialogueID);
		return false;
	}

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
		// 추가: Trade 처리
		else if (SelectedOption.SwitchToMenu == EMenuType::Trade)
		{
			ANPCCharacter* NPC = Cast<ANPCCharacter>(GetOwner());
			if (NPC)
			{
				NPC->OpenShop();
				// 대화창은 유지하거나 닫기 (선택)
				// EndDialogue(); // 대화창 닫으려면 주석 해제
			}
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
	if (!CurrentDataTable) return nullptr;

	//모든 Row를 순회하면서 DialogueID로 찾기
	TArray<FName> RowNames = CurrentDataTable->GetRowNames();

	UE_LOG(LogTemp, Error, TEXT("LoadDialogueByID: Searching for ID %d"), DialogueID);

	for (FName RowName : RowNames)
	{
		FDialogueRow* Row = CurrentDataTable->FindRow<FDialogueRow>(RowName, TEXT("LoadDialogue"));
		if (Row && Row->DialogueID == DialogueID)
		{
			UE_LOG(LogTemp, Error, TEXT("  Found! Row Name: '%s', DialogueID: %d"),
				*RowName.ToString(), Row->DialogueID);
			return Row;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("  NOT FOUND! DialogueID %d"), DialogueID);
	return nullptr;
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