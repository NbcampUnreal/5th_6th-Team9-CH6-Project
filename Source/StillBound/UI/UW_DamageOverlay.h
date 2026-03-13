#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_DamageOverlay.generated.h"

class UImage;
class UMaterialInstanceDynamic;

/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_DamageOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;

	void UpdateDamageEffect(float HealthPercent);

protected:

	UPROPERTY(meta = (BindWidget))
	UImage* DamageImage;

	UPROPERTY()
	UMaterialInstanceDynamic* DamageMID;

	FTimerHandle PulseTimer;

	float PulseTime = 0.f;

	void UpdatePulse();
};
