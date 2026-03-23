#include "UI/UW_PickupText.h"
#include "UI/UW_UIHUD.h"
#include "Components/PanelWidget.h"
#include "GameFramework/HUD.h"
#include "Components/TextBlock.h"

void UUW_PickupText::SetPickupText(const FText& InText)
{
    if (!PickupText)
        return;

    FString RawText = InText.ToString().TrimStartAndEnd();

    FString ItemName;
    int32 Amount = 1;

    int32 PlusIndex;
    if (RawText.FindLastChar(TEXT('+'), PlusIndex))
    {
        ItemName = RawText.Left(PlusIndex).TrimEnd();

        FString AmountString = RawText.Mid(PlusIndex + 1).TrimStartAndEnd();
        Amount = FCString::Atoi(*AmountString);

        if (Amount <= 0)
        {
            Amount = 1;
        }
    }
    else
    {
        ItemName = RawText;
    }

    BaseText = ItemName;
    StackCount = Amount;

    UpdateText();
}

void UUW_PickupText::UpdateText()
{
    if (PickupText)
    {
        FString NewText = FString::Printf(TEXT("%s +%d"), *BaseText, StackCount);
        PickupText->SetText(FText::FromString(NewText));
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

void UUW_PickupText::AddStack(int32 Amount)
{
    StackCount += Amount;

    UpdateText();
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
