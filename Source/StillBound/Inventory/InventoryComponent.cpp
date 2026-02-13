#include "Inventory/InventoryComponent.h"
#include "InventoryComponent.h"
#include "Data/ItemData.h"
#include "Items/ItemBase.h"

UInventoryComponent::UInventoryComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	InventorySlots.SetNum(InventorySlotsCapacity);
	HotbarContents.SetNum(HotbarSlotsCapacity);

}

UItemBase* UInventoryComponent::FindMatchingItem(UItemBase* ItemIn) const
{
	if (!ItemIn) return nullptr;

	for (const TObjectPtr<UItemBase>& Ptr : InventorySlots)
	{
		if (Ptr.Get() == ItemIn)
			return ItemIn;
	}
	return nullptr;

}

UItemBase* UInventoryComponent::FindNextItemByID(UItemBase* ItemIn) const
{
	if (!ItemIn) return nullptr;

	for (const TObjectPtr<UItemBase>& Ptr : InventorySlots)
	{
		UItemBase* It = Ptr.Get();
		if (It && It->ID == ItemIn->ID)
			return It;
	}
	return nullptr;

}

UItemBase* UInventoryComponent::FindNextPartialStack(UItemBase* ItemIn) const
{
	if (!ItemIn) return nullptr;

	for (const TObjectPtr<UItemBase>& Ptr : InventorySlots)
	{
		UItemBase* It = Ptr.Get();
		if (It && It->ID == ItemIn->ID && !It->IsFullItemStack())
			return It;
	}
	return nullptr;

}

UItemBase* UInventoryComponent::FindNextPartialStackInHotbar(UItemBase* ItemIn) const
{
	if (!ItemIn) return nullptr;
	
	for (const TObjectPtr<UItemBase>& Ptr : HotbarContents)
	{
		UItemBase* It = Ptr.Get();
		if (It && It->ID == ItemIn->ID && !It->IsFullItemStack())
		{
			return It;
		}
	}
	return nullptr;
}

int32 UInventoryComponent::FindFirstEmptyInventorySlot() const
{
	for (int32 i = 0; i < InventorySlots.Num(); ++i)
	{
		if (InventorySlots[i] == nullptr)
			return i;
	}
	return INDEX_NONE;
}

int32 UInventoryComponent::FindFirstEmptyHotbarIndex() const
{
	for (int32 i = 0; i < HotbarContents.Num(); ++i)
	{
		if (HotbarContents[i] == nullptr)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

int32 UInventoryComponent::CalculateWeightAddAmount(UItemBase* ItemIn, int32 RequestedAddAmount)
{
	const int32 WeightMaxAddAmount = FMath::FloorToInt((GetWeightCapacity() - InventoryTotalWeight) / ItemIn->GetItemSingleWeight());
	if (WeightMaxAddAmount >= RequestedAddAmount)
	{
		return RequestedAddAmount;
	}

	return WeightMaxAddAmount;
}

int32 UInventoryComponent::CalculateNumberForFullStack(UItemBase* StackableItem, int32 InitialRequestedAddAmount)
{
	const int32 AddAmountToMakeFullStack = StackableItem->NumericData.MaxStackSize - StackableItem->Quantity;

	return FMath::Min(InitialRequestedAddAmount, AddAmountToMakeFullStack);
}

void UInventoryComponent::RemoveSingleInstanceOfItem(UItemBase* ItemToRemove)
{
	if (!ItemToRemove) return;

	for (int32 i = 0; i < InventorySlots.Num(); ++i)
	{
		if (InventorySlots[i].Get() == ItemToRemove)
		{
			InventorySlots[i] = nullptr;
			OnInventoryUpdated.Broadcast();
			return;
		}
	}
}

int32 UInventoryComponent::RemoveAmountOfItem(UItemBase* ItemIn, int32 DesiredAmountToRemove)
{
	const int32 ActualAmountToRemove = FMath::Min(DesiredAmountToRemove, ItemIn->Quantity);

	ItemIn->SetQuantity(ItemIn->Quantity - ActualAmountToRemove);

	InventoryTotalWeight -= ActualAmountToRemove * ItemIn->GetItemSingleWeight();

	OnInventoryUpdated.Broadcast();

	return ActualAmountToRemove;
}

void UInventoryComponent::SplitExistingStack(UItemBase* ItemIn, const int32 AmountToSplit)
{
	if (FindFirstEmptyInventorySlot() != INDEX_NONE)
	{
		RemoveAmountOfItem(ItemIn, AmountToSplit);
		AddNewItem(ItemIn, AmountToSplit);
	}
}

FItemAddResult UInventoryComponent::HandleNonStackableItems(UItemBase* InputItem)
{
	// check if in the input item has valid weight
	if (FMath::IsNearlyZero(InputItem->GetItemSingleWeight()) || InputItem->GetItemSingleWeight() < 0)
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Could not add {0} to the inventory. Item has invalid weight value."), InputItem->TextData.Name));
	}

	// will the item weight overflow weight capacity
	if (InventoryTotalWeight + InputItem->GetItemSingleWeight() > GetWeightCapacity())
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Could not add {0} to the inventory. Item would overflow weight limit."), InputItem->TextData.Name));
	}

	// adding one more item would overflow slot capacity
	if (FindFirstEmptyInventorySlot() == INDEX_NONE)
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("Could not add {0} to the inventory. All Inventory slots are full."), InputItem->TextData.Name));
	}

	AddNewItem(InputItem, 1);
	return FItemAddResult::AddedAll(1, FText::Format(FText::FromString("Successfully added a single {0} to the inventory."), InputItem->TextData.Name));
}

int32 UInventoryComponent::HandleStackableItems(UItemBase* ItemIn, int32 RequestedAddAmount)
{
	if (RequestedAddAmount <= 0 || FMath::IsNearlyZero(ItemIn->GetItemStackWeight()))
	{
		// invalid item data
		return 0;
	}

	int32 AmountToDistribute = RequestedAddAmount;

	// check if the input item already exists in the inventory and is not a full stack
	UItemBase* ExistingItemStack = FindNextPartialStack(ItemIn);

	// distribute time stack over existing stacks
	while (ExistingItemStack)
	{
		// calculate how many of the existing item would be needed to make the next full stack
		const int32 AmountToMakeFullStack = CalculateNumberForFullStack(ExistingItemStack, AmountToDistribute);
		// calculate how many of the AmountToMakeFullStack can actually be carried based on weight capacity
		const int32 WeightLimitAddMount = CalculateWeightAddAmount(ExistingItemStack, AmountToMakeFullStack);

		// as long as the remaining amount of the item does noet overflow weight capacity
		if (WeightLimitAddMount > 0)
		{
			// adjust the existing items stack quantity and inventory total weight
			ExistingItemStack->SetQuantity(ExistingItemStack->Quantity + WeightLimitAddMount);
			InventoryTotalWeight += ExistingItemStack->GetItemSingleWeight() * WeightLimitAddMount;

			// adjust the count to be distributed
			AmountToDistribute -= WeightLimitAddMount;

			ItemIn->SetQuantity(AmountToDistribute);

			// TODO: Refine this logic since going over weight capacity should not ever be possible
			// if max weight capacity is reached, no need to run the loop again
			if (InventoryTotalWeight >= InventoryWeightCapacity)
			{
				OnInventoryUpdated.Broadcast();
				return RequestedAddAmount - AmountToDistribute;

			}
		}
		else if (WeightLimitAddMount <= 0)
		{
			if (AmountToDistribute != RequestedAddAmount)
			{
				// this block will be reached if distributing an item across multiple stacks
				// and the weight limit is hit during that process
				OnInventoryUpdated.Broadcast();
				return RequestedAddAmount - AmountToDistribute;
			}

			return 0;
		}

		if (AmountToDistribute <= 0)
		{
			// all of the input item was distributed across existing stacks
			OnInventoryUpdated.Broadcast();
			return RequestedAddAmount;
		}

		// check if there is still another valid partial stack of the input item
		ExistingItemStack = FindNextPartialStack(ItemIn);
	}

	// no more partial stacks found, check if a new stack can be added
	if (FindFirstEmptyInventorySlot() != INDEX_NONE)
	{
		// attempt to add as many from the remaining item quantity that can fit inventory weight capacity
		const int32 WeightLimitAddAmount = CalculateWeightAddAmount(ItemIn, AmountToDistribute);

		if (WeightLimitAddAmount > 0)
		{
			// if there is still more item to distribute, but weight limit has been reached
			if (WeightLimitAddAmount < AmountToDistribute)
			{
				// adjust the input item and add a new stack with as many as can be held
				AmountToDistribute -= WeightLimitAddAmount;
				ItemIn->SetQuantity(AmountToDistribute);

				// create a copy since only a partial stack is being added
				AddNewItem(ItemIn->CreateItemCopy(), WeightLimitAddAmount);
				return RequestedAddAmount - AmountToDistribute;
			}

			// otherwise, the full remainder of the stack can be added
			AddNewItem(ItemIn, AmountToDistribute);
			return RequestedAddAmount;
		}
	}

	OnInventoryUpdated.Broadcast();
	return RequestedAddAmount - AmountToDistribute;
}

FItemAddResult UInventoryComponent::HandleAddItem(UItemBase* InputItem)
{
	if (GetOwner())
	{
		const int32 InitialRequestedAddAmount = InputItem->Quantity;

		// handle non-stackable items
		if (!InputItem->NumericData.bIsStackable)
		{
			return HandleNonStackableItems(InputItem);
		}

		// handle stackable
		const int32 StackableAmountAdded = HandleStackableItems(InputItem, InitialRequestedAddAmount);

		if (StackableAmountAdded == InitialRequestedAddAmount)
		{
			return FItemAddResult::AddedAll(InitialRequestedAddAmount, FText::Format(
				FText::FromString("Succeesfully added {0} {1} to the inventory."), InitialRequestedAddAmount, InputItem->TextData.Name));
		}

		if (StackableAmountAdded < InitialRequestedAddAmount && StackableAmountAdded>0)
		{
			return FItemAddResult::AddedPartial(StackableAmountAdded, FText::Format(
				FText::FromString("Partial amount of {0} added to the inventory. Number added = {1}"), InputItem->TextData.Name, StackableAmountAdded));
		}

		if (StackableAmountAdded <= 0)
		{
			return FItemAddResult::AddedNone(FText::Format(
				FText::FromString("Couldn't add {0} to the inventory. No remaining inventory slots, or invalid item."), InputItem->TextData.Name));
		}
	}

	check(false);
	return FItemAddResult::AddedNone(FText::FromString("TryAddItem fallthrough error. GetOwner() check somehow failed."));
}

FItemAddResult UInventoryComponent::HandleAddItem_AutoHotbarFirst(UItemBase* InputItem)
{
	if (!GetOwner() || !InputItem)
	{
		return FItemAddResult::AddedNone(FText::FromString("Invalid Owner or item."));
	}

	const int32 InitialRequestedAddAmount = InputItem->Quantity;


	/// Non-stackable : 핫바 빈칸 먼저///
	if (!InputItem->NumericData.bIsStackable)
	{
		if (FMath::IsNearlyZero(InputItem->GetItemSingleWeight()) || InputItem->GetItemSingleWeight() < 0)
		{
			return FItemAddResult::AddedNone(FText::Format(FText::FromString("Could not add {0}. Invalid weight."), InputItem->TextData.Name));
		}

		if (InventoryTotalWeight + InputItem->GetItemSingleWeight() > GetWeightCapacity())
		{
			return FItemAddResult::AddedNone(FText::Format(FText::FromString("Could not add {0}. Weight limit reached."), InputItem->TextData.Name));
		}

		const int32 EmptyIdx = FindFirstEmptyHotbarIndex();
		if (EmptyIdx != INDEX_NONE)
		{
			UItemBase* NewItem = nullptr;

			if (InputItem->bIsCopy || InputItem->bIsPickup)
			{
				NewItem = InputItem;
				NewItem->ResetItemFlags();
			}
			else
			{
				NewItem = InputItem->CreateItemCopy();
			}

			NewItem->OwningInventory = this;
			NewItem->SetQuantity(1);

			HotbarContents[EmptyIdx] = NewItem;
			InventoryTotalWeight += NewItem->GetItemSingleWeight();

			OnHotbarUpdated.Broadcast();
			OnInventoryUpdated.Broadcast();

			return FItemAddResult::AddedAll(1, FText::Format(FText::FromString("Added {0} to hotbar slot {1}."), InputItem->TextData.Name, FText::AsNumber(EmptyIdx + 1)));
		}

		//핫바가 꽉 찼으면 기존 인벤 로직으로
		return HandleNonStackableItems(InputItem);
	}

	/// stackable: 핫바 부분스택 -> 핫바 빈칸 -> 인벤 ///
	int32 AmountToDistribute = InitialRequestedAddAmount;

	//핫바 부분 스택 채우기
	UItemBase* ExistingHotbarStack = FindNextPartialStackInHotbar(InputItem);
	while (ExistingHotbarStack && AmountToDistribute > 0)
	{
		const int32 AmountToMakeFullStack = CalculateNumberForFullStack(ExistingHotbarStack, AmountToDistribute);
		const int32 WeightLimitAddAmount = CalculateWeightAddAmount(ExistingHotbarStack, AmountToMakeFullStack);

		if (WeightLimitAddAmount <= 0)
		{
			break;
		}

		ExistingHotbarStack->SetQuantity(ExistingHotbarStack->Quantity + WeightLimitAddAmount);
		InventoryTotalWeight += ExistingHotbarStack->GetItemSingleWeight() * WeightLimitAddAmount;

		AmountToDistribute -= WeightLimitAddAmount;

		ExistingHotbarStack = FindNextPartialStackInHotbar(InputItem);
	}

	//핫바 빈칸에 새 스택 만들기
	while (AmountToDistribute > 0)
	{
		const int32 EmptyIdx = FindFirstEmptyHotbarIndex();
		if (EmptyIdx == INDEX_NONE) break;

		const int32 ToFullStack = FMath::Min(AmountToDistribute, InputItem->NumericData.MaxStackSize);
		const int32 WeightLimitAddAmount = CalculateWeightAddAmount(InputItem, ToFullStack);

		if (WeightLimitAddAmount <= 0) break;

		// New 스택아이템 만들기 : 부분이면 copy 필요
		UItemBase* NewStack = nullptr;
		if (WeightLimitAddAmount < AmountToDistribute)
		{
			//부분 스택일 가능성 있으니 copy로 안정
			NewStack = InputItem->CreateItemCopy();
		}
		else
		{
			//다 넣을 거면 원본 써도되지만 월드 픽업 여부에 따라 안전하게 copy
			NewStack = InputItem->bIsPickup ? InputItem : InputItem->CreateItemCopy();
		}

		NewStack->ResetItemFlags();
		NewStack->OwningInventory = this;
		NewStack->SetQuantity(WeightLimitAddAmount);

		HotbarContents[EmptyIdx] = NewStack;
		InventoryTotalWeight += NewStack->GetItemSingleWeight() * WeightLimitAddAmount;

		AmountToDistribute -= WeightLimitAddAmount;
	}

	//핫바에 넣은 만큼
	const int32 AddedToHotbar = InitialRequestedAddAmount - AmountToDistribute;

	if (AddedToHotbar > 0)
	{
		OnHotbarUpdated.Broadcast();
		OnInventoryUpdated.Broadcast();
	}

	//남은 수량은 기존 인벤 로직으로
	if (AmountToDistribute > 0)
	{
		//InputItem 수량을 남은 만큼으로 맞추고 HandleAddItem 호출
		InputItem->SetQuantity(AmountToDistribute);
		const FItemAddResult InvResult = HandleAddItem(InputItem);

		//결과 합치기(핫바+인벤)
		const int32 TotalAdded = AddedToHotbar + InvResult.ActualAmountAdded;

		if (TotalAdded <= 0)
		{
			return FItemAddResult::AddedNone(FText::Format(FText::FromString("Couldn't add {0} (hotbar full + inventory full/weight)."), InputItem->TextData.Name));
		}

		if (TotalAdded < InitialRequestedAddAmount)
		{
			return FItemAddResult::AddedPartial(TotalAdded, FText::Format(FText::FromString("Added {0} partially (hotbar+inventory)."), InputItem->TextData.Name));
		}

		return FItemAddResult::AddedAll(TotalAdded, FText::Format(FText::FromString("Added{0} fully (hotbar+Inventory)."), InputItem->TextData.Name));
	}

	//핫바에서 전부 소화
	return FItemAddResult::AddedAll(InitialRequestedAddAmount, FText::Format(FText::FromString("Added {0} to hotbar."), InputItem->TextData.Name));
}

void UInventoryComponent::AddNewItem(UItemBase* Item, const int32 AmountToAdd)
{
	const int32 EmptySlot = FindFirstEmptyInventorySlot();
	if (EmptySlot == INDEX_NONE)
	{
		OnInventoryUpdated.Broadcast();
		return;
	}

	UItemBase* NewItem = nullptr;

	if (Item->bIsCopy || Item->bIsPickup)
	{
		NewItem = Item;
		NewItem->ResetItemFlags();
	}
	else
	{
		NewItem = Item->CreateItemCopy();
	}

	NewItem->OwningInventory = this;
	NewItem->SetQuantity(AmountToAdd);

	InventorySlots[EmptySlot] = NewItem;

	InventoryTotalWeight += NewItem->GetItemStackWeight();
	OnInventoryUpdated.Broadcast();
}

int32 UInventoryComponent::GetOccupiedSlotCount() const
{
	int32 Count = 0;
	for (const TObjectPtr<UItemBase>& Ptr : InventorySlots)
	{
		if (Ptr != nullptr) ++Count;
	}
	return Count;
}

UItemBase* UInventoryComponent::GetItemAtIndex(int32 Index) const
{
	return InventorySlots.IsValidIndex(Index) ? InventorySlots[Index].Get() : nullptr;
}

int32 UInventoryComponent::RemoveAmountAtIndex(int32 Index, int32 Quantity)
{
	if (!InventorySlots.IsValidIndex(Index)) return 0;

	UItemBase* Item = InventorySlots[Index].Get();
	if (!Item) return 0;

	const int32 Removed = FMath::Min(Quantity, Item->Quantity);
	Item->SetQuantity(Item->Quantity - Removed);

	InventoryTotalWeight -= Removed * Item->GetItemSingleWeight();

	if (Item->Quantity <= 0)
	{
		InventorySlots[Index] = nullptr;
	}

	OnInventoryUpdated.Broadcast();
	return Removed;
}

UItemBase* UInventoryComponent::GetItemInContainer(ESlotContainer InContainer, int32 Index) const
{
	const TArray<TObjectPtr<UItemBase>>& Arr = (InContainer == ESlotContainer::Inventory) ? InventorySlots : HotbarContents;

	return Arr.IsValidIndex(Index) ? Arr[Index].Get() : nullptr;
}

int32 UInventoryComponent::RemoveAmountInContainer(ESlotContainer InContainer, int32 Index, int32 Quantity)
{
	TArray<TObjectPtr<UItemBase>>& Arr = (InContainer == ESlotContainer::Inventory) ? InventorySlots : HotbarContents;

	if (!Arr.IsValidIndex(Index)) return 0;

	UItemBase* Item = Arr[Index].Get();
	if (!Item) return 0;

	const int32 Removed = FMath::Min(Quantity, Item->Quantity);
	Item->SetQuantity(Item->Quantity - Removed);

	InventoryTotalWeight -= Removed * Item->GetItemSingleWeight();

	if (Item->Quantity <= 0)
	{
		Arr[Index] = nullptr;
	}

	if (InContainer == ESlotContainer::Inventory)
	{
		OnInventoryUpdated.Broadcast();
	}
	
	else
	{
		OnHotbarUpdated.Broadcast();
	}

	OnInventoryUpdated.Broadcast();

	return Removed;
}

bool UInventoryComponent::MoveSlotItem(ESlotContainer FromContainer, int32 FromIndex, ESlotContainer ToContainer, int32 ToIndex, bool bAllowSwap)
{
	TArray<TObjectPtr<UItemBase>>& FromArr = (FromContainer == ESlotContainer::Inventory) ? InventorySlots : HotbarContents;
	TArray<TObjectPtr<UItemBase>>& ToArr = (ToContainer == ESlotContainer::Inventory) ? InventorySlots : HotbarContents;

	if (!FromArr.IsValidIndex(FromIndex) || !ToArr.IsValidIndex(ToIndex)) return false;
	if (FromArr[FromIndex] == nullptr) return false;

	if (FromContainer == ToContainer && FromIndex == ToIndex) return false;

	if (ToArr[ToIndex] == nullptr)
	{
		ToArr[ToIndex] = FromArr[FromIndex];
		FromArr[FromIndex] = nullptr;
	}
	else
	{
		if (!bAllowSwap) return false;

		Swap(FromArr[FromIndex], ToArr[ToIndex]);
	}

	if (FromContainer == ESlotContainer::Inventory || ToContainer == ESlotContainer::Inventory)
		OnInventoryUpdated.Broadcast();

	if (FromContainer == ESlotContainer::Hotbar || ToContainer == ESlotContainer::Hotbar)
		OnHotbarUpdated.Broadcast();

	UE_LOG(LogTemp, Warning, TEXT("[MoveSlotItem] From(%d,%d) To(%d,%d)  FromNum=%d ToNum=%d"),
		(int32)FromContainer, FromIndex, (int32)ToContainer, ToIndex,
		FromArr.Num(), ToArr.Num());

	return true;
}



