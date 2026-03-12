#include "UI/UW_HPBar.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UUW_HPBar::SetHP(float Current, float Max)
{
	if (!PB_Health || Max <= 0.f) return;

	PB_Health->SetPercent(Current / Max);

	if (TXT_HP)
	{
		FString HPText = FString::Printf(TEXT("%.0f / %.0f"), Current, Max);
		TXT_HP->SetText(FText::FromString(HPText));
	}
}
