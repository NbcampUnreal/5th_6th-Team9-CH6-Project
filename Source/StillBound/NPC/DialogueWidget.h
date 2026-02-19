// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueData.h"
#include "DialogueOptionButton.h"
#include "DialogueWidget.generated.h"

/**
 * 
 */

class UImage;
class UTextBlock;
class UVerticalBox;
class UBotton;
class UDialogueComponent;
class UDialogueOptionButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueOptionSelected, int32, OptionIndex);

UCLASS()
class STILLBOUND_API UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//대화 데이터 표시
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ShowDialogue(const FDialogueRow& DialogueData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnMenuTypeChanged(EMenuType NewMenuType);

	//대화 달기
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void CloseDialogue();

	//선택지 클릭 이벤
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void HandleOptionSelected(int32 OptionIndex);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDialogueOptionSelected OnOptionClicked;

	//대화 컴포넌트 설정
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SetDialogueComponent(UDialogueComponent* Component);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> OptionButtonClass;

	//바인드 위젯 변수명

	UPROPERTY(meta = (BindWidget))
	class UImage* IMG_Profile;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TXT_NPCName;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TXT_DialogueContent;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* VB_OptionList;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	UDialogueComponent* DialogueComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FDialogueRow CurrentDialogueData;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_MenuTitle;

private:
	UPROPERTY()
	TArray<UDialogueOptionButton*> OptionButtonPool;
};
