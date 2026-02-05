// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueOptionButton.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptionClicked, int32, OptionIndex);

UCLASS()
class STILLBOUND_API UDialogueOptionButton : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void InitializeOption(int32 Index, FText Content);

	FOnOptionClicked OnOptionClicked;

protected:
	UFUNCTION()
	void OnBtnClicked();

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Option;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TXT_OptionContent;

private:
	int32 MyOptionIndex;

};
