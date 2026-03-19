#include "UI/UW_UIHUD.h"
#include "UI/UW_HPBar.h"
#include "UI/UW_StaminaBar.h"
#include "UI/UW_ExpBar.h"
#include "UI/UW_Minimap.h"
#include "UI/UW_BossHPbar.h"
#include "UI/Inventory/HotbarPanel.h"
#include "Inventory/InventoryComponent.h"
#include "Components/Image.h"

void UUW_UIHUD::NativeConstruct()
{
	Super::NativeConstruct();

	SetBuildGuideVisibile(false);
}

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

void UUW_UIHUD::SetBuildGuideVisibile(bool bVisible)
{
	if (BuildGuideImage)
	{
		BuildGuideImage->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}
