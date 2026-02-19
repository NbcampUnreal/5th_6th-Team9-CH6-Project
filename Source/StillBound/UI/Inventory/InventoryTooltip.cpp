#include "UI/Inventory/InventoryTooltip.h"
#include "UI/Inventory/InventoryItemSlot.h"
#include "Components/TextBlock.h"
#include "Items/ItemBase.h"

void UInventoryTooltip::NativeConstruct()
{
	Super::NativeConstruct();

	if (!InventorySlotBeingHovered)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	UItemBase* ItemBeingHovered = InventorySlotBeingHovered->GetItemReference();
	if (!ItemBeingHovered)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

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
		break;
	default:;
	}

	ItemName->SetText(ItemBeingHovered->TextData.Name);
	DamageValue->SetText(FText::AsNumber(ItemBeingHovered->ItemStatistics.DamageValue));
	ArmorRating->SetText(FText::AsNumber(ItemBeingHovered->ItemStatistics.ArmorRating));
	UsageText->SetText(ItemBeingHovered->TextData.UsageText);
	ItemDescription->SetText(ItemBeingHovered->TextData.Description);

	const FString WeightInfo = { "Weight: " + FString::SanitizeFloat(ItemBeingHovered->GetItemStackWeight()) };

	StackWeight->SetText(FText::FromString(WeightInfo));

	if (ItemBeingHovered->NumericData.bIsStackable)
	{
		const FString StackInfo = { "Max Stack size: " + FString::FromInt(ItemBeingHovered->NumericData.MaxStackSize) };

		MaxStackSize->SetText(FText::FromString(StackInfo));
	}
	else
	{
		MaxStackSize->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInventoryTooltip::RefreshFromSlot()
{
	if (!InventorySlotBeingHovered)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	UItemBase* Item = InventorySlotBeingHovered->GetItemReference();
	if (!Item)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	SetVisibility(ESlateVisibility::Visible);

	ItemName->SetText(Item->TextData.Name);
	DamageValue->SetText(FText::AsNumber(Item->ItemStatistics.DamageValue));
	ArmorRating->SetText(FText::AsNumber(Item->ItemStatistics.ArmorRating));
	UsageText->SetText(Item->TextData.UsageText);
	ItemDescription->SetText(Item->TextData.Description);
	
	const FString WeightInfo = FString::Printf(TEXT("Weight: %s"), *FString::SanitizeFloat(Item->GetItemStackWeight()));

	StackWeight->SetText(FText::FromString(WeightInfo));
}