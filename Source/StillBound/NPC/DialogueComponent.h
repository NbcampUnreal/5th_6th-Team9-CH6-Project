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

//대화 시스템 컴포넌트

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
    // 각 메뉴별 DataTable
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
    UDataTable* MainMenuDataTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
    UDataTable* DialogueDataTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
    UDataTable* QuestDataTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
    UDataTable* TradeDataTable;

	// 시작 대화 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 StartDialogueID = 1;

    // === 대화 제어 ===
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool StartDialogue(AActor* Interactor);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool SelectOption(int32 OptionIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void GoToDialogue(int32 DialogueID);

    // 메뉴 전환
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SwitchToMenu(EMenuType MenuType);

    // 메인 메뉴로 돌아가기
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ReturnToMainMenu();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue")
    bool CheckCondition(const FDialogueCondition& Condition);

    // === 상태 조회 ===
    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    FDialogueRow GetCurrentDialogue() const { return CurrentDialogue; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    EMenuType GetCurrentMenuType() const { return CurrentMenuType; }

    // === 이벤트 ===
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
    //대화이력
    TArray<int32> DialogueHistory;

    // 대화 데이터 로드
    FDialogueRow* LoadDialogueByID(int32 DialogueID);
    UDataTable* GetDataTableForMenu(EMenuType MenuType);
};
