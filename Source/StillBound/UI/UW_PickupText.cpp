#include "UI/UW_PickupText.h"
#include "UI/UW_UIHUD.h"
#include "Components/PanelWidget.h"
#include "GameFramework/HUD.h"
#include "Components/TextBlock.h"

void UUW_PickupText::SetPickupText(const FText& InText)
{
	if (PickupText)
	{
		PickupText->SetText(InText);
	}
}

void UUW_PickupText::StartLifeTimer(float LifeTime)
{
    UE_LOG(LogTemp, Warning, TEXT("Pickup Timer Start"));

    if (FadeOut)
    {
        PlayAnimation(FadeOut);
    }

    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UWorld* World = PC->GetWorld())
        {
            World->GetTimerManager().SetTimer(
                RemoveTimer,
                this,
                &UUW_PickupText::RemoveSelf,
                LifeTime,
                false
            );
        }
    }
}

void UUW_PickupText::RemoveSelf()
{
    if (UPanelWidget* Parent = GetParent())
    {
        if (UUW_UIHUD* HUD = Parent->GetTypedOuter<UUW_UIHUD>())
        {
            RemoveFromParent();

            if (UWorld* World = GetWorld())
            {
                World->GetTimerManager().SetTimerForNextTick([HUD]()
                    {
                        HUD->CheckPickupPanel();
                    });
            }

            return;
        }
    }

    RemoveFromParent();
}
