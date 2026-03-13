#include "UI/UW_RoundProgressBar.h"

void UUW_RoundProgressBar::NativeConstruct()
{
    Super::NativeConstruct();

    if (ProgressImage)
    {
        ProgressMaterial = ProgressImage->GetDynamicMaterial();

        if (ProgressMaterial)
        {
            ProgressMaterial->SetScalarParameterValue("Percent", 0.f);
        }
    }
}

void UUW_RoundProgressBar::SetPercent(float Percent)
{
    if (!ProgressMaterial)
        return;

    Percent = FMath::Clamp(Percent, 0.f, 1.f);
    ProgressMaterial->SetScalarParameterValue("Percent", Percent);
}

void UUW_RoundProgressBar::SetRemainingTime(float Remaining)
{
    if (!TimeText)
        return;

    FString Str = FString::Printf(TEXT("%.1fs"), Remaining);
    TimeText->SetText(FText::FromString(Str));
}
