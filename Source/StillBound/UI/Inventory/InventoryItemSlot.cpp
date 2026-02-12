#include "UI/Inventory/InventoryItemSlot.h"
#include "UI/Inventory/InventoryTooltip.h"
#include "UI/Inventory/DragItemVisual.h"
#include "UI/Inventory/ItemDragDropOperation.h"
#include "Items/ItemBase.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Inventory/InventoryComponent.h"

void UInventoryItemSlot::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (ToolTipClass)
	{
		ToolTip = CreateWidget<UInventoryTooltip>(this, ToolTipClass);
		if (ToolTip)
		{
			ToolTip->InventorySlotBeingHovered = this;
			SetToolTip(ToolTip);
		}
	}

	//UInventoryTooltip* ToolTip = CreateWidget<UInventoryTooltip>(this, ToolTipClass);
	//ToolTip->InventorySlotBeingHovered = this;
	//SetToolTip(ToolTip);
}

void UInventoryItemSlot::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemReference)
	{
		switch (ItemReference->ItemQuality)
		{
		case EItemQuality::Common:
			ItemBorder->SetBrushColor(FLinearColor::Gray);
			break;
		case EItemQuality::Rare:
			ItemBorder->SetBrushColor(FLinearColor::Green);
			break;
		case EItemQuality::Unique:
			ItemBorder->SetBrushColor(FLinearColor(0.f, 0.4f, 0.75f));
			break;
		case EItemQuality::Legendary:
			ItemBorder->SetBrushColor(FLinearColor(1.f, 0.45f, 0.f));
			break;
		default:;
		}

		ItemIcon->SetBrushFromTexture(ItemReference->AssetData.Icon);

		if (ItemReference->NumericData.bIsStackable)
		{
			ItemQuantity->SetText(FText::AsNumber(ItemReference->Quantity));
		}
		else
		{
			ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

FReply UInventoryItemSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (!ItemReference) return Reply.Unhandled();
		return Reply.Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	// SubMenu on right click will happen here

	return Reply.Unhandled();
}

void UInventoryItemSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (!ItemReference) return;

	if (ToolTip)
	{
		ToolTip->RefreshFromSlot();
		SetToolTip(ToolTipWidget);      
	}
}

void UInventoryItemSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UInventoryItemSlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!ItemReference)
	{
		OutOperation = nullptr;
		return;
	}

	if (DragItemVisualClass)
	{
		const TObjectPtr<UDragItemVisual> DragVisul = CreateWidget<UDragItemVisual>(this, DragItemVisualClass);
		DragVisul->ItemIcon->SetBrushFromTexture(ItemReference->AssetData.Icon);
		DragVisul->ItemBorder->SetBrushColor(ItemBorder->GetBrushColor());

		ItemReference->NumericData.bIsStackable 
			? DragVisul->ItemQuantity->SetText(FText::AsNumber(ItemReference->Quantity)) 
			: DragVisul->ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);

		UItemDragDropOperation* DragItemOperation = NewObject<UItemDragDropOperation>();
		DragItemOperation->SourceItem = ItemReference;         
		DragItemOperation->SourceInventory = InventoryRef;           /*DragItemOperation->SourceInventory = ItemReference->OwningInventory;*/
		DragItemOperation->SourceContainer = Container;
		DragItemOperation->SourceIndex = SlotIndex;

		DragItemOperation->DefaultDragVisual = DragVisul;
		DragItemOperation->Pivot = EDragPivot::TopLeft;

		OutOperation = DragItemOperation;



	}
}

bool UInventoryItemSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	/*return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);*/

	const UItemDragDropOperation* Drag = Cast<UItemDragDropOperation>(InOperation);
	if (!Drag || !InventoryRef) return false;

	return InventoryRef->MoveSlotItem(
		Drag->SourceContainer, Drag->SourceIndex,
		Container, SlotIndex,
		/*bAllowSwap=*/true
	);
}

void UInventoryItemSlot::InitSlot(ESlotContainer InContainer, int32 InIndex, UInventoryComponent* InInv)
{
	Container = InContainer;
	SlotIndex = InIndex;
	InventoryRef = InInv;
}

void UInventoryItemSlot::SetItemReference(UItemBase* ItemIn)
{
	ItemReference = ItemIn;

	if (!ItemIcon) return;

	if (!ItemReference)
	{
		ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
		if (ItemQuantity)
			ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);

		SetToolTip(nullptr);

		return;
	}

	ItemIcon->SetVisibility(ESlateVisibility::Visible);

	if (ItemReference->AssetData.Icon)
	{
		ItemIcon->SetBrushFromTexture(ItemReference->AssetData.Icon);
	}

	if (ItemQuantity)
	{
		const int32 Qty = ItemReference->Quantity;
		if (Qty > 1)
		{
			ItemQuantity->SetText(FText::AsNumber(Qty));
			ItemQuantity->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
	if (ToolTip)
	{
		SetToolTip(ToolTip);
	}
	
	ToolTip->RefreshFromSlot();
}
