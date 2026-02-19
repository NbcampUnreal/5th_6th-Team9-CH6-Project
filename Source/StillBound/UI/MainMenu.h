// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"

class APlayerCharacter_SB;
class UInventoryPanel;

/**
 * 
 */
UCLASS()
class STILLBOUND_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

public:
	UPROPERTY()
	TObjectPtr<APlayerCharacter_SB> PlayerCharacter;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryPanel> WBP_InventoryPanel;

	TObjectPtr<UInventoryPanel> GetInventoryPanel() const { return WBP_InventoryPanel; };

};
