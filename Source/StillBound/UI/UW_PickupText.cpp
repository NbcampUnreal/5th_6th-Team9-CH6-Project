#include "UI/UW_PickupText.h"
#include "UI/UW_UIHUD.h"
#include "Components/PanelWidget.h"
#include "GameFramework/HUD.h"
#include "Components/TextBlock.h"

void UUW_PickupText::SetPickupText(const FText& ItemName, int32 Amount)
{
    if (!PickupText)
        return;

    BaseText = ItemName.ToString();

    StackCount = Amount;

    UpdateText();
}

void UUW_PickupText::UpdateText()
{
    if (!PickupText)
        return;

    FText FinalText = FText::Format(
        FText::FromString("{0} +{1}"),
        FText::FromString(BaseText),
        FText::AsNumber(StackCount) 
    );

    PickupText->SetText(FinalText);
}


void UUW_PickupText::StartLifeTimer(float LifeTime)
{
    UE_LOG(LogTemp, Warning, TEXT("Pickup Timer Start"));

    if (FadeOut)
    {
        StopAnimation(FadeOut);
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

void UUW_PickupText::AddStack(int32 Amount)
{
    StackCount += Amount;
    UpdateText();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RemoveTimer);
    }

    StartLifeTimer(2.f);
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
