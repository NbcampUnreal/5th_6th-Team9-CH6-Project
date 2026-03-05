#include "UI/UW_Minimap.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UUW_Minimap::NativeConstruct()
{
    Super::NativeConstruct();

    if (!MiniMapImage) return;

    UObject* Resource = MiniMapImage->GetBrush().GetResourceObject();
    UMaterialInterface* BaseMaterial = Cast<UMaterialInterface>(Resource);

    if (!BaseMaterial) return;

    MiniMapMID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
    MiniMapImage->SetBrushFromMaterial(MiniMapMID);

    MiniMapMID->SetVectorParameterValue(
        FName("MapOffset"),
        FLinearColor(0.5f, 0.5f, 0, 0)
    );

    MiniMapMID->SetScalarParameterValue(
        FName("Zoom"),
        CurrentZoom
    );

    if (PingIcon)
    {
        PingIcon->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UUW_Minimap::UpdateMapOffset(const FVector2D& PlayerUV)
{
    if (!MiniMapMID) return;

    FVector2D Offset = PlayerUV;
    Offset.Y = 1.f - Offset.Y;

    MiniMapMID->SetVectorParameterValue(
        FName("MapOffset"),
        FLinearColor(Offset.X, Offset.Y, 0, 0)
    );
}

void UUW_Minimap::UpdatePlayerIconRotation(float Yaw)
{
	if (!PlayerIcon) return;

	PlayerIcon->SetRenderTransformAngle(Yaw);
}

void UUW_Minimap::UpdatePing(const FVector2D& PingUV, const FVector2D& PlayerUV)
{
    if (!PingIcon) return;

    FVector2D Relative = PingUV - PlayerUV;

    FVector2D Pos;
    Pos.X = Relative.X * MapSize * CurrentZoom;
    Pos.Y = -Relative.Y * MapSize * CurrentZoom;

    const float Radius = MapSize * 0.5f;

    float Dist = Pos.Size();

    if (Dist > Radius)
    {
        Pos = Pos.GetSafeNormal() * Radius;
    }

    PingIcon->SetRenderTranslation(Pos);
    PingIcon->SetVisibility(ESlateVisibility::Visible);
}

void UUW_Minimap::ClearPing()
{
    if (PingIcon)
    {
        PingIcon->SetVisibility(ESlateVisibility::Hidden);
    }
}


