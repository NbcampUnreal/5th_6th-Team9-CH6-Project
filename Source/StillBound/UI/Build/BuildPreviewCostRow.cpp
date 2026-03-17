#include "UI/Build/BuildPreviewCostRow.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"

void UBuildPreviewCostRow::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemIconSizeBox)
	{
		ItemIconSizeBox->SetWidthOverride(32.f);
		ItemIconSizeBox->SetHeightOverride(32.f);
	}
}

void UBuildPreviewCostRow::InitRow(UTexture2D* InIcon, const FText& InItemName, int32 Have, int32 Need)
{
	if (ItemIconImage)
	{
		if (InIcon)
		{
			ItemIconImage->SetBrushFromTexture(InIcon, true);
			ItemIconImage->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ItemIconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (ItemNameText)
	{
		ItemNameText->SetText(InItemName);
	}

	if (CountText)
	{
		const FString CountString = FString::Printf(TEXT("%d / %d"), Have, Need);
		CountText->SetText(FText::FromString(CountString));

		if (Have < Need)
		{
			CountText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
		}
		else
		{
			CountText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
	}
}

