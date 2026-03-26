// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestCompleteWidget.generated.h"

/**
 * 
 */
UCLASS()
class STILLBOUND_API UQuestCompleteWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void InitializePopup(const FText& QuestName, int32 RewardGold);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* TXT_QuestName; 

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TXT_RewardGold;

	UPROPERTY(meta = (BindWidget))
	class UButton* BTN_Confirm;

	UFUNCTION()
	void OnConfirmClicked();	
};
