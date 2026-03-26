// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueData.h"
#include "DialogueComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, const FDialogueRow&, DialogueData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueUpdated, const FDialogueRow&, NewDialogueData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenuChanged, EMenuType, NewMenuType);

//��ȭ �ý��� ������Ʈ

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STILLBOUND_API UDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDialogueComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
    // �� �޴��� DataTable
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
    UDataTable* MainMenuDataTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
    UDataTable* DialogueDataTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
    UDataTable* QuestDataTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
    UDataTable* TradeDataTable;

	// ���� ��ȭ ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 StartDialogueID = 1;

    // === ��ȭ ���� ===
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectOption(int32 OptionIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void GoToDialogue(int32 DialogueID);

    // �޴� ��ȯ
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SwitchToMenu(EMenuType MenuType);

    // ���� �޴��� ���ư���
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ReturnToMainMenu();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    bool CheckCondition(const FDialogueCondition& Condition);

    // === ���� ��ȸ ===
    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    FDialogueRow GetCurrentDialogue() const { return CurrentDialogue; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    EMenuType GetCurrentMenuType() const { return CurrentMenuType; }

    // === �̺�Ʈ ===
    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueUpdated OnDialogueUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnMenuChanged OnMenuChanged;
    
private:
    bool bIsDialogueActive = false;
    FDialogueRow CurrentDialogue;
    AActor* CurrentInteractor = nullptr;

    EMenuType CurrentMenuType = EMenuType::None;
    UDataTable* CurrentDataTable = nullptr;
    //��ȭ�̷�
    TArray<int32> DialogueHistory;

    // ��ȭ ������ �ε�
    FDialogueRow* LoadDialogueByID(int32 DialogueID);
    UDataTable* GetDataTableForMenu(EMenuType MenuType);
};
