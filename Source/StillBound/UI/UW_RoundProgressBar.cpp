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
