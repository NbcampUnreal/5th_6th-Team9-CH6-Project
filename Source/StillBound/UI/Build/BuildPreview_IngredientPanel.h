#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuildPreview_IngredientPanel.generated.h"

UCLASS()
class STILLBOUND_API UBuildPreview_IngredientPanel : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	bool bCanPlayerByLocation = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	bool bHasEnoughCost = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	FText CurrentBuildStatusText;

};
