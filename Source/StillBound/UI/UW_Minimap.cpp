#include "UI/UW_Minimap.h"
#include "Components/Image.h"
#include "Engine/TextureRenderTarget2D.h"

void UUW_Minimap::SetMiniMapTexture(UTextureRenderTarget2D* InRenderTarget)
{
	if (!MiniMapImage || !InRenderTarget) return;

	MiniMapImage->SetBrushResourceObject(InRenderTarget);
}
