#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_Minimap.generated.h"

class UImage;
class UMaterialInstanceDynamic;

/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_Minimap : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;

	void UpdateMapOffset(const FVector2D& PlayerUV);

	void UpdatePlayerIconRotation(float Yaw);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MiniMapImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerIcon;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MiniMapMID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	float CurrentZoom = 2.f;

};
