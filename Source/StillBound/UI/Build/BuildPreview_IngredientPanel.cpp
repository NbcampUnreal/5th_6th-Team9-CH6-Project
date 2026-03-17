#include "UI/Build/BuildPreview_IngredientPanel.h"
#include "UI/Build/BuildPreviewCostRow.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Engine/Texture2D.h"

void UBuildPreview_IngredientPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlacementStateText)
	{
		PlacementStateText->SetVisibility(ESlateVisibility::Collapsed);
		PlacementStateText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

void UBuildPreview_IngredientPanel::UpdateIngredientList(const TArray<FBuildPreviewCostUIData>& InCosts)
{
	if (!IngredientListBox || !CostRowClass) return;

	SetVisibility(ESlateVisibility::Visible);

	IngredientListBox->ClearChildren();

	for (const FBuildPreviewCostUIData& CostData : InCosts)
	{
		UBuildPreviewCostRow* RowWidget = CreateWidget<UBuildPreviewCostRow>(GetOwningPlayer(), CostRowClass);
		if (!RowWidget) continue;

		RowWidget->InitRow(CostData.Icon, CostData.ItemName, CostData.Have, CostData.Need);
		IngredientListBox->AddChild(RowWidget);
	}
}

void UBuildPreview_IngredientPanel::ShowPlacementStateMessage(const FText& InMessage, float Duration)
{
	if (!PlacementStateText) return;

	PlacementStateText->SetText(InMessage);
	PlacementStateText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
	PlacementStateText->SetVisibility(ESlateVisibility::Visible);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PlacementStateTimerHandle);

		GetWorld()->GetTimerManager().SetTimer(PlacementStateTimerHandle, this, &ThisClass::HidePlacementStateMessage, Duration, false);
	}
}

void UBuildPreview_IngredientPanel::HidePlacementStateMessage()
{
	if (PlacementStateText)
	{
		PlacementStateText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

