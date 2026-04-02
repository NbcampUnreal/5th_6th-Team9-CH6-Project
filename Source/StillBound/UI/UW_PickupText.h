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
	void SetPickupText(const FText& ItemName, int32 Amount);

	void StartLifeTimer(float LifeTime = 2.f);

	void AddStack(int32 Amount);

	FString GetBaseText() const { return BaseText; }

protected:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PickupText;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* FadeOut;

private:

	FTimerHandle RemoveTimer;

	int32 StackCount = 1;

	FString BaseText;

	void UpdateText();

	UFUNCTION()
	void RemoveSelf();
};
