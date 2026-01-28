#include "UI/UW_UIHUD.h"
#include "UI/UW_HPBar.h"
#include "UI/UW_StaminaBar.h"


void UUW_UIHUD::SetHP(float Current, float Max)
{
	if (!HPBar) return;

	HPBar->SetHP(Current, Max);
}

void UUW_UIHUD::SetStamin(float Current, float Max)
{
	if (StaminaBar)
	{
		StaminaBar->SetStamina(Current, Max);
	}
}
