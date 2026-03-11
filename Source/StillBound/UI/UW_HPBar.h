#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_HPBar.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_HPBar : public UUserWidget
{
	GENERATED_BODY()
	
public:  
	UFUNCTION(BlueprintCallable)
	void SetHP(float Current, float Max);

protected:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* PB_Health;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TXT_HP;
};
