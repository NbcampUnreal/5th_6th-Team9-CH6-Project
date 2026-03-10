#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"

class APlayerCharacter_SB;
class UInventoryPanel;
class UCraftingPanel;
class UInventoryComponent;

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

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCraftingPanel> WBP_CraftingPanel;

	TObjectPtr<UInventoryPanel> GetInventoryPanel() const { return WBP_InventoryPanel; };

	void ShowInventoryOnly();
	void ShowCrafting(UInventoryComponent* InInventory);
};
