#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_UIHUD.generated.h"

class UUW_HPBar;
class UUW_StaminaBar;
/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_UIHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void SetHP(float Current, float Max);

	UFUNCTION(BlueprintCallable)
	void SetStamin(float Current, float Max);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UUW_HPBar> HPBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UUW_StaminaBar> StaminaBar;
};
