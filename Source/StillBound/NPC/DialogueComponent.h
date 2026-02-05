// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueData.h"
#include "DialogueComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, const FDialogueRow&, DialogueData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueUpdated, const FDialogueRow&, NewDialogueData);

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
	
	// === 대화 데이터 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	UDataTable* DialogueDataTable;

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

    // === 상태 조회 ===
    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    FDialogueRow GetCurrentDialogue() const { return CurrentDialogue; }

    // === 이벤트 ===
    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Dialogue")
    FOnDialogueUpdated OnDialogueUpdated;

    
private:
    bool bIsDialogueActive = false;
    FDialogueRow CurrentDialogue;
    AActor* CurrentInteractor = nullptr;

    // 대화 데이터 로드
    FDialogueRow* LoadDialogueByID(int32 DialogueID);
};
