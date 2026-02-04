#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_Minimap.generated.h"

class UImage;
class UTextureRenderTarget2D;
/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_Minimap : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetMiniMapTexture(UTextureRenderTarget2D* InRenderTarget);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MiniMapImage;
};
