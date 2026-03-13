#include "Items/ItemBase.h"
#include "Inventory/InventoryComponent.h"

UItemBase::UItemBase() : bIsCopy(false), bIsPickup(false)
{
}

void UItemBase::ResetItemFlags()
{
	bIsCopy = false;
	bIsPickup = false;
}

UItemBase* UItemBase::CreateItemCopy()
{
	UItemBase* ItemCopy = NewObject<UItemBase>(StaticClass());

	ItemCopy->ID = this->ID;
	ItemCopy->Quantity = this->Quantity;
	ItemCopy->ItemQuality = this->ItemQuality;
	ItemCopy->ItemType = this->ItemType;
	ItemCopy->TextData = this->TextData;
	ItemCopy->NumericData = this->NumericData;
	ItemCopy->ItemStatistics = this->ItemStatistics;
	ItemCopy->AssetData = this->AssetData;
	ItemCopy->bIsCopy = true;

	ItemCopy->EquipWeaponClass = this->EquipWeaponClass; //weapon

	ItemCopy->ConsumableEffectClass = this->ConsumableEffectClass;      //수정
	ItemCopy->ConsumableSetByCallerTag = this->ConsumableSetByCallerTag; //수정

	return ItemCopy;
}

void UItemBase::SetQuantity(const int32 NewQuantity)
{
	if (NewQuantity != Quantity)
	{
		Quantity = FMath::Clamp(NewQuantity, 0, NumericData.bIsStackable ? NumericData.MaxStackSize : 1);

		if (OwningInventory)
		{
			if (Quantity <= 0)
			{
				this->OwningInventory->RemoveSingleInstanceOfItem(this);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ItemBase OwningInventory was null!"));
		}
	}
}

void UItemBase::Use(ABaseCharacter_SB* Character)
{
}

