#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Public/Data/CraftingRecipeRow.h"
#include "CraftingPanel.generated.h"

class UScrollBox;
class UVerticalBox;
class UWrapBox;
class UTextBlock;
class UButton;
class UImage;
class UInventoryComponent;
class UTexture2D;
class URecipeEntryWidget;

UCLASS()
class STILLBOUND_API UCraftingPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Init(UInventoryComponent* InInv);

	UFUNCTION(BlueprintCallable)
	void SelectRecipe(FName RecipeID);

	UTexture2D* FindItemIconByID(FName ItemID) const;
	FText FindItemDisplayNameByID(FName ItemID) const;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UScrollBox> RecipeScroll;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWrapBox> RecipeWrapBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ResultText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> IngredientBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CraftButton;

	UPROPERTY(EditDefaultsOnly, Category = "Crafting|UI")
	TSubclassOf<UUserWidget> RecipeEntryClass;

	UPROPERTY(EditDefaultsOnly, Category = "Crafting|UI")
	TSubclassOf<UUserWidget> IngredientRowClass;

	UFUNCTION()
	void OnCraftClicked();

	void RebuildRecipeList();
	void RefreshDetail();

private:
	UPROPERTY()
	TObjectPtr<UInventoryComponent> Inv;

	bool bBoundInventoryEvent = false;
	FName SelectedRecipeID = NAME_None;

};
