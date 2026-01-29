#include "UI/UW_ExpBar.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UUW_ExpBar::SetExp(float Current, float Required)
{
	if (!PB_Exp || Required <= 0.f) return;
	PB_Exp->SetPercent(Current / Required);
}

void UUW_ExpBar::SetLevel(int32 InLevel)
{
	if (!TXT_Level) return;

	TXT_Level->SetText(FText::FromString(FString::Printf(TEXT("Lv.%d"), InLevel)));
}
