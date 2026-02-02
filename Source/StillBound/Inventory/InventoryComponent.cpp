#include "Inventory/InventoryComponent.h"
#include "InventoryComponent.h"
#include "Data/ItemData.h"

UInventoryComponent::UInventoryComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

	Capacity = 24;
}

void UInventoryComponent::BroadcastInventoryUpdated() const
{
	if (OnInventoryUpdated.IsBound())
	{
		OnInventoryUpdated.Broadcast();
	}
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Slots.Num() == 0)
	{
		InitializeSlots(Capacity);
	}
}

void UInventoryComponent::InitializeSlots(int32 InSlotCount)
{
	InSlotCount = FMath::Max(1, InSlotCount);
	Slots.SetNum(InSlotCount);

	for (FInventorySlot& Slot : Slots)
	{
		Slot.ItemID = NAME_None;
		Slot.Quantity = 0;
	}

	BroadcastInventoryUpdated();
}

bool UInventoryComponent::AddItem(FName ItemID, int32 Amount, int32& OutRemaining)
{
	if (Amount <= 0 || nullptr == ItemDataTable)
	{
		return false;
	}

	const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemID, TEXT("InventoryComponent::AddItem"));
	if (nullptr == ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("아이템 데이터 테이블에 존재하지 않는 ID가 있습니다."));
		return false;
	}

	int32 AmountToAdd = Amount;

	//1. 중첩 가능한 수량만큼 합치기

	if (ItemData->MaxStack > 1)
	{
		FInventorySlot* ExistingSlot = FindStackableSlot(ItemID, ItemData);
		if (ExistingSlot)
		{
			int32 SpaceLeft = ItemData->MaxStack - ExistingSlot->Quantity;
			int32 AmountToFill = FMath::Min(AmountToAdd, SpaceLeft);
			ExistingSlot->Quantity += AmountToFill;
		}
	}

	// 남은 수량 빈 슬롯에 찾아 새로 넣기
	while (AmountToAdd > 0)
	{
		FInventorySlot* EmptySlot = FindEmptySlot();
		if (nullptr == EmptySlot)
		{
			UE_LOG(LogTemp, Warning, TEXT("인벤토리 공간이 부족합니다."));
			return false;
		}

		int32 AmountToFill = FMath::Min(AmountToAdd, ItemData->MaxStack);
		EmptySlot->ItemID = ItemID;
		EmptySlot->Quantity = AmountToFill;
	}

	BroadcastInventoryUpdated();

	return true;
}

bool UInventoryComponent::RemoveItem(FName ItemID, int32 Amount, int32& OutRemaining)
{
	if (Amount <= 0 || nullptr == ItemDataTable)
	{
		return false;
	}

	const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemID, TEXT("InventoryCompontn::RemoveItem"));
	if (nullptr == ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("아이템 데이터 테이블에 존재하지 않는 ID가 있습니다."));
		return false;
	}

	int32 AmountToRemove = Amount;

	while (AmountToRemove > 0)
	{
		FInventorySlot* ExistingSlot = FindStackableSlot(ItemID, ItemData);
		if (nullptr == ExistingSlot)
		{
			UE_LOG(LogTemp, Warning, TEXT("제거할 아이템 수량이 부족합니다."));
			break;
		}
		
		int32 AmountToTake = FMath::Min(AmountToRemove, ExistingSlot->Quantity);
		ExistingSlot->Quantity -= AmountToTake;
		AmountToRemove -= AmountToTake;

		if (ExistingSlot->Quantity <= 0)
		{
			ExistingSlot->Clear();
		}
	}

	BroadcastInventoryUpdated();

	return true;
}

FInventorySlot* UInventoryComponent::FindStackableSlot(FName ItemID, const FItemDataRow* ItemData)
{
	for (FInventorySlot& Slot : Slots)
	{
		if (false == Slot.IsEmpty() && Slot.ItemID == ItemID && Slot.Quantity <= ItemData->MaxStack)
		{
			return &Slot;
		}
	}

	return nullptr;
}

FInventorySlot* UInventoryComponent::FindEmptySlot()
{
	for (FInventorySlot& Slot : Slots)
	{
		if (Slot.IsEmpty())
		{
			return &Slot;
		}
	}

	return nullptr;
}




