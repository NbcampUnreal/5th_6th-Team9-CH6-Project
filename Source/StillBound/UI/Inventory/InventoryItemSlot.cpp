#include "UI/Inventory/InventoryItemSlot.h"
#include "UI/Inventory/InventoryTooltip.h"
#include "UI/Inventory/DragItemVisual.h"
#include "UI/Inventory/ItemDragDropOperation.h"
#include "UI/MainMenu.h"
#include "Items/ItemBase.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Inventory/InventoryComponent.h"
#include "Character/PlayerController_SB.h"

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
		SetToolTip(ToolTip);      
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

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (auto* SBPC = Cast<APlayerController_SB>(PC))
		{
			if (SBPC->UIManager && SBPC->UIManager->GetMainMenuWidget())
			{
				SBPC->UIManager->GetMainMenuWidget()->EnableDropCatcher(true);
			}
		}
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
		/*DragItemOperation->SourceContainer = Container;*/   //추후 보관상자 구현할때...
		DragItemOperation->SourceItem = ItemReference;
		DragItemOperation->SourceContainer = Container;
		DragItemOperation->SourceIndex = SlotIndex;

		DragItemOperation->DefaultDragVisual = DragVisul;
		DragItemOperation->Pivot = EDragPivot::TopLeft;

		OutOperation = DragItemOperation;



	}
}

bool UInventoryItemSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const UItemDragDropOperation* Drag = Cast<UItemDragDropOperation>(InOperation);
	if (!Drag || !InventoryRef)
	{
		DisableDropCatcher();
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("[SlotDrop] From: %d(%d) To: %d(%d)"),
		(int32)Drag->SourceContainer, Drag->SourceIndex, (int32)Container, SlotIndex);

	DisableDropCatcher();

	bool bSuccess = InventoryRef->MoveSlotItem(
		Drag->SourceContainer, 
		Drag->SourceIndex,
		Container, 
		SlotIndex, 
		true);

	return bSuccess;
}

void UInventoryItemSlot::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	DisableDropCatcher();
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
		ItemIcon->SetBrushFromTexture(nullptr);
		ItemIcon->SetVisibility(ESlateVisibility::Hidden);

		if (ItemQuantity)
		{
			ItemQuantity->SetText(FText::GetEmpty());
			ItemQuantity->SetVisibility(ESlateVisibility::Hidden);
		}

		if (ItemBorder)
		{
			ItemBorder->SetVisibility(ESlateVisibility::Visible);
		}

		SetToolTip(nullptr);
		return;
	}

	if (ItemBorder)
	{
		ItemBorder->SetVisibility(ESlateVisibility::Visible);
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
		ToolTip->RefreshFromSlot();
	}
}

void UInventoryItemSlot::DisableDropCatcher()
{
	if (auto* PC = GetOwningPlayer())
	{
		if (auto* MyPC = Cast<APlayerController_SB>(PC))
		{
			if (MyPC->UIManager && MyPC->UIManager->GetMainMenuWidget())
			{
				MyPC->UIManager->GetMainMenuWidget()->EnableDropCatcher(false);
			}
		}
	}
}