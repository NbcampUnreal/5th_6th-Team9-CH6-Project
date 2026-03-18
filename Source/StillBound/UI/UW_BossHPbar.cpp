#include "UI/UW_BossHPbar.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "UW_UIHUD.h"

void UBossHPbar::NativeConstruct()
{
	Super::NativeConstruct();

    SetVisibility(ESlateVisibility::Hidden);
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
    if (HPBar)
    {
        HPBar->SetPercent(Percent);
    }
}
