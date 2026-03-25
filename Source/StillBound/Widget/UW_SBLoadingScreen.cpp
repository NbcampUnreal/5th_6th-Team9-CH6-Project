#include "Widget/UW_SBLoadingScreen.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UUW_SBLoadingScreen::SetLoadingProgress(float InPercent)
{
    const float Clamped = FMath::Clamp(InPercent, 0.f, 1.f);

    if (ProgressBar_Loading)
    {
        ProgressBar_Loading->SetPercent(Clamped);
    }

    if (Text_LoadingPercent)
    {
        const int32 PercentInt = FMath::RoundToInt(Clamped * 100.f);
        Text_LoadingPercent->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentInt)));
    }
}