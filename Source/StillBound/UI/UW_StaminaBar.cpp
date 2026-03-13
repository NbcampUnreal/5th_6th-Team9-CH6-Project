#include "UI/UW_StaminaBar.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UUW_StaminaBar::SetStamina(float Current, float Max)
{
	if (!PB_Stamina || Max <= 0.f) return;

	PB_Stamina->SetPercent(Current / Max);

	if (TXT_Stamina)
	{
		FString StaminaText = FString::Printf(TEXT("%.0f / %.0f"), Current, Max);
		TXT_Stamina->SetText(FText::FromString(StaminaText));
	}
}