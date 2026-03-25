#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_Crosshair.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_Crosshair : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;

	void SetCrosshairVisible(bool bVisible);

protected:

	UPROPERTY(meta = (BindWidget))
	UImage* CrosshairImage;
};
