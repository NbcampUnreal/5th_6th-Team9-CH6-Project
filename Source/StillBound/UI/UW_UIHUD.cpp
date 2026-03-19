#include "UI/UW_UIHUD.h"
#include "UI/UW_HPBar.h"
#include "UI/UW_StaminaBar.h"
#include "UI/UW_ExpBar.h"
#include "UI/UW_Minimap.h"
#include "UI/UW_BossHPbar.h"
#include "UI/UW_PickupText.h"
#include "Components/Border.h"
#include "UI/Inventory/HotbarPanel.h"
#include "Components/VerticalBox.h"
#include "Inventory/InventoryComponent.h"

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

void UUW_UIHUD::AddPickupLog(const FText& Text)
{
	if (!PickupLogBox || !PickupTextClass)
	{
		return;
	}

	UUW_PickupText* PickupWidget =
		CreateWidget<UUW_PickupText>(GetOwningPlayer(), PickupTextClass);

	if (!PickupWidget)
	{
		return;
	}

	PickupPanel->SetVisibility(ESlateVisibility::Visible);


	PickupWidget->SetPickupText(Text);
	PickupWidget->StartLifeTimer(2.f);

	PickupLogBox->InsertChildAt(0, PickupWidget);
}

void UUW_UIHUD::CheckPickupPanel()
{
	UE_LOG(LogTemp, Warning, TEXT("CheckPickupPanel Called"));

	if (!PickupLogBox || !PickupPanel)
	{
		UE_LOG(LogTemp, Warning, TEXT("PickupLogBox or PickupPanel NULL"));
		return;
	}

	int32 Count = PickupLogBox->GetChildrenCount();

	UE_LOG(LogTemp, Warning, TEXT("Pickup Children Count: %d"), Count);

	if (Count == 0)
	{
		PickupPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UUW_UIHUD::SetBossName(const FText& Name)
{
	if (!BossHP) return;

	BossHP->SetVisibility(ESlateVisibility::Visible);
	BossHP->SetBossName(Name);
}

void UUW_UIHUD::SetBossHP(float Current, float Max)
{
	if (!BossHP) return;

	BossHP->SetHPPercent(Current / Max);
}

void UUW_UIHUD::HideBossHP()
{
	if (!BossHP) return;

	BossHP->SetVisibility(ESlateVisibility::Hidden);
}

void UUW_UIHUD::InitInventory(UInventoryComponent* InInv)
{
	if (HotbarPanel)
	{
		HotbarPanel->InitWithInventory(InInv);
	}
}

void UUW_UIHUD::SetSelectedHotbarIndex(int32 Index)
{
	if (HotbarPanel)
	{
		HotbarPanel->SetSelectedIndex(Index);
	}
}
