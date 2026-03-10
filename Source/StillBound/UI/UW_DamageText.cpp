#include "UI/UW_DamageText.h"
#include "Components/TextBlock.h"

void UUW_DamageText::NativeConstruct()
{
	Super::NativeConstruct();

	if (Anim_Float)
	{
		PlayAnimation(Anim_Float);
	}
}

void UUW_DamageText::SetDamage(int32 Damage)
{
	if (!Text_Damage)
	{
		return;
	}

	Text_Damage->SetText(FText::AsNumber(Damage));
}
