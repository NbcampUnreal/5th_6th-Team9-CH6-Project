#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Containers/Ticker.h"
#include "SBGameInstance.generated.h"

class UUserWidget;
class UUW_SBLoadingScreen;

UCLASS()
class STILLBOUND_API USBGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;
    virtual void Shutdown() override;

    UFUNCTION(BlueprintCallable)
    void RequestLoadingScreenOnce();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loading")
    TSubclassOf<UUserWidget> LoadingScreenClass;

private:
    UFUNCTION()
    void BeginLoadingScreen(const FString& MapName);

    UFUNCTION()
    void EndLoadingScreen(UWorld* LoadedWorld);

    bool TickLoadingProgress(float DeltaTime);

private:
    UPROPERTY()
    TObjectPtr<UUW_SBLoadingScreen> ActiveLoadingWidget;

    FString LoadingMapName;
    FTSTicker::FDelegateHandle LoadingTickerHandle;

    bool bSkipFirstLoadScreen = true;
    bool bShowLoadingScreenOnce = false;
    float FakeLoadingProgress = 0.f;
};