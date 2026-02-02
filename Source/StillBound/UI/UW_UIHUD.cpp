#include "UI/UW_UIHUD.h"
#include "UI/UW_HPBar.h"
#include "UI/UW_StaminaBar.h"
#include "UI/UW_ExpBar.h"

void UUW_UIHUD::SetHP(float Current, float Max)
{
	if (!HPBar) return;

	HPBar->SetHP(Current, Max);
}

void UUW_UIHUD::SetStamina(float Current, float Max)
{
	//UE_LOG(LogTemp, Warning, TEXT("[UIHUD] SetStamin: %f / %f"), Current, Max);

	if (StaminaBar)
	{
		StaminaBar->SetStamina(Current, Max);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[UIHUD] StaminaBar is null (BindWidget failed?)"));
	}
}

void UUW_UIHUD::SetExp(float Current, float Required)
{
	if (ExpBar)
	{
		ExpBar->SetExp(Current, Required);
	}
}

void UUW_UIHUD::SetLevel(int32 Level)
{
	if (ExpBar)
	{
		ExpBar->SetLevel(Level);
	}
}
