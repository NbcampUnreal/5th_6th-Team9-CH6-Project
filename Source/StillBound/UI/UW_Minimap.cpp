#include "UI/UW_Minimap.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UUW_Minimap::NativeConstruct()
{
	Super::NativeConstruct();

	if (!MiniMapImage)
	{
		UE_LOG(LogTemp, Error, TEXT("[Minimap] MiniMapImage is null"));
		return;
	}

	UObject* Resource = MiniMapImage->GetBrush().GetResourceObject();
	UMaterialInterface* BaseMaterial = Cast<UMaterialInterface>(Resource);

	if (!BaseMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("[Minimap] Brush resource is NOT a material"));
		return;
	}

	MiniMapMID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	if (!MiniMapMID)
	{
		UE_LOG(LogTemp, Error, TEXT("[Minimap] Failed to create MID"));
		return;
	}

	MiniMapImage->SetBrushFromMaterial(MiniMapMID);

	MiniMapMID->SetVectorParameterValue(
		FName("MapOffset"),
		FLinearColor(0.5f, 0.5f, 0.f, 0.f)
	);

	UE_LOG(LogTemp, Warning, TEXT("[Minimap] MID CREATED & APPLIED"));
}

void UUW_Minimap::UpdateMapOffset(const FVector2D& PlayerUV)
{
    if (!MiniMapMID) return;

    FVector2D Offset = PlayerUV;

	Offset.Y = 1.f - Offset.Y;

	Offset.X = FMath::Clamp(Offset.X, 0.f, 1.f);
	Offset.Y = FMath::Clamp(Offset.Y, 0.f, 1.f);

    MiniMapMID->SetVectorParameterValue(
        FName("MapOffset"),
        FLinearColor(Offset.X, Offset.Y, 0.f, 0.f)
    );

    MiniMapMID->SetScalarParameterValue(
        FName("Zoom"),
        CurrentZoom
    );
}

void UUW_Minimap::UpdatePlayerIconRotation(float Yaw)
{
	if (!PlayerIcon) return;

	PlayerIcon->SetRenderTransformAngle(Yaw);
}



