#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_SBLoadingScreen.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class STILLBOUND_API UUW_SBLoadingScreen : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void SetLoadingProgress(float InPercent);

protected:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> ProgressBar_Loading;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_LoadingPercent;
};