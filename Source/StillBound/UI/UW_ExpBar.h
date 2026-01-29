#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ExpBar.generated.h"

class UProgressBar;
class UTextBlock;
/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_ExpBar : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void SetExp(float Current, float Required);

	UFUNCTION(BlueprintCallable)
	void SetLevel(int32 InLevel);
protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_Exp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_Level;
};
