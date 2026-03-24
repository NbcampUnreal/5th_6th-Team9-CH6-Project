#include "UI/UW_BossHPbar.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "UW_UIHUD.h"

void UBossHPbar::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Hidden);

	if (HPBar)
	{
		HPBar->SetPercent(1.f);
	}

	if (HPBar_Delay)
	{
		HPBar_Delay->SetPercent(1.f);
	}
}

void UBossHPbar::SetBossName(const FText& Name)
{
    if (BossName)
    {
        BossName->SetText(Name);
    }
}

void UBossHPbar::SetHPPercent(float Percent)
{
	if (!HPBar || !HPBar_Delay)
	{
		return;
	}

	HPBar->SetPercent(Percent);
	TargetDelayPercent = Percent;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DelayStartHandle);
		World->GetTimerManager().ClearTimer(DelayMoveHandle);

		World->GetTimerManager().SetTimer(
			DelayStartHandle,
			this,
			&UBossHPbar::StartDelayBarUpdate,
			1.0f,   
			false
		);
	}
}

void UBossHPbar::StartDelayBarUpdate()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DelayMoveHandle,
			this,
			&UBossHPbar::UpdateDelayHP,
			0.02f,
			true
		);
	}
}

void UBossHPbar::UpdateDelayHP()
{
	if (!HPBar_Delay || !HPBar)
	{
		return;
	}

	const float Current = HPBar_Delay->GetPercent();

	const float NewPercent = FMath::FInterpTo(
		Current,
		TargetDelayPercent,
		GetWorld()->GetDeltaSeconds(),
		4.f
	);

	HPBar_Delay->SetPercent(NewPercent);

	if (FMath::IsNearlyEqual(NewPercent, TargetDelayPercent, 0.005f))
	{
		HPBar_Delay->SetPercent(TargetDelayPercent);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DelayMoveHandle);
		}
	}
}
