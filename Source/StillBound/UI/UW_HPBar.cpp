#include "UI/UW_HPBar.h"
#include "Components/ProgressBar.h"

void UUW_HPBar::SetHP(float Current, float Max)
{
	if (!PB_Health || Max <= 0.f) return;

	PB_Health->SetPercent(Current / Max);
}
