#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "IngredientRowWidget.generated.h"

class UTextBlock;
class UImage;
class UTexture2D;
class USizeBox;

UCLASS()
class STILLBOUND_API UIngredientRowWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void InitRow(const FText& InDisplayName, int32 Have, int32 Need, UTexture2D* IconTexture);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> IconSizeBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountText;
};
