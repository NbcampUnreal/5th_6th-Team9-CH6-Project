#include "GameInstance/SBGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "MoviePlayer.h"
#include "Widget/UW_SBLoadingScreen.h"
#include "UObject/UObjectGlobals.h"
#include "Containers/Ticker.h"

void USBGameInstance::Init()
{
    Super::Init();

    FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &USBGameInstance::BeginLoadingScreen);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &USBGameInstance::EndLoadingScreen);
}

void USBGameInstance::Shutdown()
{
    FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

    if (LoadingTickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(LoadingTickerHandle);
        LoadingTickerHandle.Reset();
    }

    Super::Shutdown();
}

void USBGameInstance::BeginLoadingScreen(const FString& MapName)
{
    if (IsRunningDedicatedServer())
    {
        return;
    }

    if (bSkipFirstLoadScreen)
    {
        bSkipFirstLoadScreen = false;
        return;
    }

    if (!bShowLoadingScreenOnce)
    {
        return;
    }

    bShowLoadingScreenOnce = false;

    if (!LoadingScreenClass)
    {
        return;
    }

    LoadingMapName = MapName;
    FakeLoadingProgress = 0.f;

    ActiveLoadingWidget = CreateWidget<UUW_SBLoadingScreen>(this, LoadingScreenClass);
    if (!ActiveLoadingWidget)
    {
        return;
    }

    ActiveLoadingWidget->SetLoadingProgress(0.f);

    FLoadingScreenAttributes LoadingScreen;
    LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
    LoadingScreen.bWaitForManualStop = false;
    LoadingScreen.WidgetLoadingScreen = ActiveLoadingWidget->TakeWidget();

    GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);

    if (LoadingTickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(LoadingTickerHandle);
        LoadingTickerHandle.Reset();
    }

    LoadingTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &USBGameInstance::TickLoadingProgress),
        0.05f
    );
}

void USBGameInstance::EndLoadingScreen(UWorld* LoadedWorld)
{
    if (ActiveLoadingWidget)
    {
        ActiveLoadingWidget->SetLoadingProgress(1.f);
    }

    if (LoadingTickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(LoadingTickerHandle);
        LoadingTickerHandle.Reset();
    }

    FakeLoadingProgress = 0.f;
    ActiveLoadingWidget = nullptr;
    LoadingMapName.Empty();
}

bool USBGameInstance::TickLoadingProgress(float DeltaTime)
{
    if (!ActiveLoadingWidget)
    {
        return false;
    }

    const float AsyncPercent = GetAsyncLoadPercentage(FName(*LoadingMapName));

    if (AsyncPercent >= 0.f)
    {
        const float RealPercent = FMath::Clamp(AsyncPercent / 100.f, 0.f, 1.f);
        FakeLoadingProgress = FMath::Max(FakeLoadingProgress, RealPercent);
    }
    else
    {
        FakeLoadingProgress = FMath::Min(FakeLoadingProgress + DeltaTime * 0.35f, 0.9f);
    }

    ActiveLoadingWidget->SetLoadingProgress(FakeLoadingProgress);
    return true;
}

void USBGameInstance::RequestLoadingScreenOnce()
{
    bShowLoadingScreenOnce = true;
}