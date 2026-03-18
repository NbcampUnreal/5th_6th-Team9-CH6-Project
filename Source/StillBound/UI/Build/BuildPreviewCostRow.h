#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuildPreviewCostRow.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;
class USizeBox;

UCLASS()
class STILLBOUND_API UBuildPreviewCostRow : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void InitRow(UTexture2D* InIcon, const FText& InItemName, int32 Have, int32 Need);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> ItemIconSizeBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountText;
};
