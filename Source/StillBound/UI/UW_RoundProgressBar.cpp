#include "UI/UW_RoundProgressBar.h"

void UUW_RoundProgressBar::NativeConstruct()
{
    Super::NativeConstruct();

    if (ProgressImage)
    {
        ProgressMaterial = ProgressImage->GetDynamicMaterial();
    }
}

void UUW_RoundProgressBar::SetPercent(float Percent)
{
    if (ProgressMaterial)
    {
        ProgressMaterial->SetScalarParameterValue("Percent", Percent);
    }
}

void UUW_RoundProgressBar::SetRemainingTime(float Remaining)
{
    if (TimeText)
    {
        FString Str = FString::Printf(TEXT("%.1fs"), Remaining);
        TimeText->SetText(FText::FromString(Str));
    }
}
