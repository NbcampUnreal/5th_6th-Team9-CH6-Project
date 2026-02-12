#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_FullMap.generated.h"

class UImage;
class UOverlay;
/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_FullMap : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;

	void UpdatePlayerPosition(const FVector2D& PlayerUV);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> MapRoot;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MapImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerIcon;

private:

	FVector2D CachedMapSize;
};
