#include "UI/UW_Crosshair.h"
#include "Components/Image.h"

void UUW_Crosshair::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUW_Crosshair::SetCrosshairVisible(bool bVisible)
{
	if (CrosshairImage)
	{
		CrosshairImage->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}