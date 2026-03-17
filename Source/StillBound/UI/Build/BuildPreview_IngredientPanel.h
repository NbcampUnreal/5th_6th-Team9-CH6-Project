#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuildPreview_IngredientPanel.generated.h"

class UImage;
class UVerticalBox;
class UTextBlock;
class UTexture2D;
class UBuildPreviewCostRow;

USTRUCT(BlueprintType)
struct FBuildPreviewCostUIData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FText ItemName;

	UPROPERTY(BlueprintReadWrite)
	int32 Have = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 Need = 0;
};

UCLASS()
class STILLBOUND_API UBuildPreview_IngredientPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void UpdateIngredientList(const TArray<FBuildPreviewCostUIData>& InCosts);

	UFUNCTION(BlueprintCallable)
	void ShowPlacementStateMessage(const FText& InMessage, float Duration = 2.0f);

	UFUNCTION(BlueprintCallable)
	void HidePlacementStateMessage();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlacementStateText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> IngredientListBox;

	UPROPERTY(EditDefaultsOnly, Category="BuildPreview")
	TSubclassOf<UBuildPreviewCostRow> CostRowClass;

private:
	FTimerHandle PlacementStateTimerHandle;

};
