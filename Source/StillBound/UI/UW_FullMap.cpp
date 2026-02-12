#include "UI/UW_FullMap.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UUW_FullMap::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUW_FullMap::UpdatePlayerPosition(const FVector2D& PlayerUV)
{
	if (!PlayerIcon || !MapRoot) return;

	
	const FVector2D Size = MapRoot->GetCachedGeometry().GetLocalSize();

	const float X = PlayerUV.X * Size.X;
	const float Y = (1.f - PlayerUV.Y) * Size.Y; // Y ¹ÝÀü

	PlayerIcon->SetRenderTranslation(FVector2D(X, Y));
}
