#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_PickupText.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_PickupText : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetPickupText(const FText& InText);

	void StartLifeTimer(float LifeTime = 2.f);

protected:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PickupText;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* FadeOut;

private:

	FTimerHandle RemoveTimer;

	UFUNCTION()
	void RemoveSelf();
};
