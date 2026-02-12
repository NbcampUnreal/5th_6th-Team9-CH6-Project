#include "UI/Inventory/InventoryPanel.h"
#include "Character/PlayerCharacter_SB.h"
#include "Inventory/InventoryComponent.h"
#include "UI/Inventory/InventoryItemSlot.h"
#include "UI/Inventory/ItemDragDropOperation.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"


void UInventoryPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PlayerCharacter = Cast<APlayerCharacter_SB>(GetOwningPlayerPawn());

	if (PlayerCharacter)
	{
		InventoryReference = PlayerCharacter->GetInventory();
		if (InventoryReference)
		{
			InventoryReference->OnInventoryUpdated.AddUObject(this, &UInventoryPanel::RefreshInventory);
			SetInfoText();

			BuildSlotGrid();
			RefreshInventory();
		}
	}
}

void UInventoryPanel::SetInfoText() const
{
	const FString WeightInfoValue{ FString::SanitizeFloat(InventoryReference->GetInventoryTotalWeight()) + "/" + FString::SanitizeFloat(InventoryReference->GetWeightCapacity()) };

	const FString CapacityInfoValue{ FString::FromInt(InventoryReference->GetOccupiedSlotCount()) + "/" + FString::FromInt(InventoryReference->GetSlotCapacity()) };

	WeightInfo->SetText(FText::FromString(WeightInfoValue));
	CapacityInfo->SetText(FText::FromStringView(CapacityInfoValue));
}

void UInventoryPanel::BuildSlotGrid()
{
	if (!InventoryGrid || !InventorySlotClass) return;

	if (SlotWidgets.Num() > 0) return;

	InventoryGrid->ClearChildren();

	SlotWidgets.Reserve(MaxSlots);

	for (int32 Index = 0; Index < MaxSlots; ++Index)
	{
		UInventoryItemSlot* SlotWidget = CreateWidget<UInventoryItemSlot>(this, InventorySlotClass);
		SlotWidget->InitSlot(ESlotContainer::Inventory, Index, InventoryReference);
		SlotWidget->SetItemReference(nullptr);

		const int32 Row = Index / Cols;
		const int32 Col = Index % Cols;

		InventoryGrid->AddChildToUniformGrid(SlotWidget, Row, Col);
		InventoryGrid->SetSlotPadding(FMargin(4.f, 0.f));
		SlotWidgets.Add(SlotWidget);

		
	}
}

void UInventoryPanel::RefreshInventory()
{
	if (!InventoryReference || !InventorySlotClass) return;
	if (SlotWidgets.Num() == 0) return;

	const TArray<UItemBase*>& Contents = InventoryReference->GetInventorySlots();

	for (int32 i = 0; i < MaxSlots; ++i)
	{
		UInventoryItemSlot* SlotWidget = SlotWidgets[i];
		if (!SlotWidget) continue;

		UItemBase* Item = Contents.IsValidIndex(i) ? Contents[i] : nullptr;
		SlotWidget->SetItemReference(Item);
	}

	SetInfoText();
}


bool UInventoryPanel::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const UItemDragDropOperation* ItemDragDrop = Cast<UItemDragDropOperation>(InOperation);

	if (ItemDragDrop->SourceItem && InventoryReference)
	{
		UE_LOG(LogTemp, Warning, TEXT("Detected an item drop on InventoryPanel."));

		// returning true will stop the drop operation at this widget
		return true;
	}

	//returning false will cause the drop operation to fall through to underlying widgets (if any)
	return false;
}