#include "UI/Crafting/IngredientRowWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UIngredientRowWidget::InitRow(const FText& InDisplayName, int32 Have, int32 Need, UTexture2D* IconTexture)
{
	if (ItemNameText)
	{
		ItemNameText->SetText(InDisplayName);
	}

	if (CountText)
	{
		CountText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), Have, Need)));

		if (Have >= Need)
		{
			CountText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
		else
		{
			CountText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
		}
	}

	if (ItemIconImage)
	{
		if (IconTexture)
		{
			ItemIconImage->SetBrushFromTexture(IconTexture, true);
			ItemIconImage->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ItemIconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
