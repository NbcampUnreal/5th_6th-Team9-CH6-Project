#include "UI/MainMenu.h"
#include "Character/PlayerCharacter_SB.h"
#include "UI/Inventory/InventoryPanel.h"
#include "UI/Crafting/CraftingPanel.h"
#include "Inventory/InventoryComponent.h"

void UMainMenu::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UMainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerCharacter = Cast<APlayerCharacter_SB>(GetOwningPlayerPawn());

	if (WBP_CraftingPanel)
	{
		WBP_CraftingPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMainMenu::ShowInventoryOnly()
{
	if (WBP_InventoryPanel)
	{
		WBP_InventoryPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (WBP_CraftingPanel)
	{
		WBP_CraftingPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMainMenu::ShowCrafting(UInventoryComponent* InInventory)
{
	if (WBP_InventoryPanel)
	{
		WBP_InventoryPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (WBP_CraftingPanel)
	{
		WBP_CraftingPanel->SetVisibility(ESlateVisibility::Visible);
		WBP_CraftingPanel->Init(InInventory);
	}
}
