#include "UI/Inventory/InventoryTooltip.h"
#include "UI/Inventory/InventoryItemSlot.h"
#include "Components/TextBlock.h"
#include "Items/ItemBase.h"

void UInventoryTooltip::NativeConstruct()
{
	Super::NativeConstruct();

	UItemBase* ItemBeingHovered = InventorySlotBeingHovered->GetItemReference();

	switch (ItemBeingHovered->ItemType)
	{
	case EItemType::Weapon: 
		break;
	case EItemType::Armor: 
		break;
	case EItemType::Tool: 
		break;
	case EItemType::Ammo: 
		break;
	case EItemType::Material: 
		ItemType->SetText(FText::FromString("Material"));
		DamageValue->SetVisibility(ESlateVisibility::Collapsed);
		ArmorRating->SetVisibility(ESlateVisibility::Collapsed);
		UsageText->SetVisibility(ESlateVisibility::Collapsed);
		break;
	case EItemType::Building: 
		break;
	case EItemType::Consumable:
		ItemType->SetText(FText::FromString("Consumable"));
		DamageValue->SetVisibility(ESlateVisibility::Collapsed);
		ArmorRating->SetVisibility(ESlateVisibility::Collapsed);
		SellValue->SetVisibility(ESlateVisibility::Collapsed);
		break;
	default:;

		ItemName->SetText(ItemBeingHovered->TextData.Name);
		DamageValue->SetText(FText::AsNumber(ItemBeingHovered->ItemStatistics.DamageValue));
		ArmorRating->SetText(FText::AsNumber(ItemBeingHovered->ItemStatistics.ArmorRating));
		UsageText->SetText(ItemBeingHovered->TextData.UsageText);
		ItemDescription->SetText(ItemBeingHovered->TextData.Description);
		SellValue->SetText(FText::AsNumber(ItemBeingHovered->ItemStatistics.SellValue));
		StackWeight->SetText(FText::AsNumber(ItemBeingHovered->GetItemStackWeight()));

		if (ItemBeingHovered->NumericData.bIsStackable)
		{
			MaxStackSize->SetText(FText::AsNumber(ItemBeingHovered->NumericData.MaxStackSize));
		}
		else
		{
			MaxStackSize->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
