#include "UI/UW_DamageOverlay.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"


void UUW_DamageOverlay::NativeConstruct()
{
	Super::NativeConstruct();

	if (DamageImage)
	{
		DamageMID = DamageImage->GetDynamicMaterial();
	}
}

void UUW_DamageOverlay::UpdateDamageEffect(float HealthPercent)
{
	if (!DamageMID) return;

	float Radius = 0.65f;

	if (HealthPercent <= 0.1f)
	{
		Radius = 0.45f;   
	}
	else if (HealthPercent <= 0.2f)
	{
		Radius = 0.50f;   
	}
	else if (HealthPercent <= 0.3f)
	{
		Radius = 0.55f;   
	}

	if (HealthPercent <= 0.3f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			PulseTimer,
			this,
			&UUW_DamageOverlay::UpdatePulse,
			0.05f,
			true
		);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(PulseTimer);
	}



	DamageMID->SetScalarParameterValue(TEXT("Radius"), Radius);
}

void UUW_DamageOverlay::UpdatePulse()
{
	if (!DamageMID) return;

	PulseTime += 0.05f;

	float Pulse = (FMath::Sin(PulseTime * 5.f) + 1.f) * 0.5f;

	float BaseOpacity = 0.4f;

	float FinalOpacity = BaseOpacity + Pulse * 0.3f;

	DamageMID->SetScalarParameterValue(TEXT("Opacity"), FinalOpacity);
}
