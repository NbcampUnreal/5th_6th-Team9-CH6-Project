#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RecipeEntryWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UTexture2D;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnRecipeEntryClicked, FName);

UCLASS()
class STILLBOUND_API URecipeEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitEntry(FName InRecipeID, UTexture2D* InIcon, const FText& InDisplayName);

	FOnRecipeEntryClicked OnRecipeEntryClicked;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> EntryButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ResultIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ResultName;

private:
	UFUNCTION()
	void HandleClicked();

	FName RecipeID = NAME_None;


};
