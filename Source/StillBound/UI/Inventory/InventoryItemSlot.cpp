#include "UI/Inventory/InventoryItemSlot.h"
#include "UI/Inventory/InventoryTooltip.h"
#include "UI/Inventory/DragItemVisual.h"
#include "UI/Inventory/ItemDragDropOperation.h"
#include "UI/MainMenu.h"
#include "UI/UW_UIHUD.h"
#include "UI/Inventory/InventoryPanel.h"
#include "UI/Inventory/HotbarPanel.h"
#include "Items/ItemBase.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Inventory/InventoryComponent.h"
#include "Character/PlayerController_SB.h"
#include "Character/PlayerCharacter_SB.h"

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

	UE_LOG(LogTemp, Warning, TEXT("[SlotDrop] THIS=%s To=%d(%d) Inv=%s / From=%d(%d)"),
		*GetName(),
		(int32)Container, SlotIndex,
		*GetNameSafe(InventoryRef),
		Drag ? (int32)Drag->SourceContainer : -1,
		Drag ? Drag->SourceIndex : -1);


	//같은 슬롯이면 드랍은 처리된 것으로 간주(버리기 방지)
	if (Drag->SourceContainer == Container && Drag->SourceIndex == SlotIndex)
	{
		return true;
	}

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

	const UItemDragDropOperation* Drag = Cast<UItemDragDropOperation>(InOperation);
	if (!Drag) return;

	auto* PC = Cast<APlayerController_SB>(GetOwningPlayer());
	if (!PC || !PC->UIManager) return;

	const FVector2D ScreenPos = InDragDropEvent.GetScreenSpacePosition();

	// 1) 핫바 위면 버리기 금지
	if (PC->UIManager->GetHUD() &&
		PC->UIManager->GetHUD()->GetHotbarPanel() &&
		PC->UIManager->GetHUD()->GetHotbarPanel()->GetCachedGeometry().IsUnderLocation(ScreenPos))
	{
		return;
	}

	// 2) 메뉴가 떠있고 메뉴 영역 안이면 버리지 않음
	if (UMainMenu* Menu = PC->UIManager->GetMainMenuWidget())
	{
		if (Menu->GetVisibility() != ESlateVisibility::Collapsed)
		{
			// 메뉴 전체가 아니라 "인벤 패널" 위면 버리기 금지
			if (Menu->GetInventoryPanel() &&
				Menu->GetInventoryPanel()->GetCachedGeometry().IsUnderLocation(ScreenPos))
			{
				return;
			}
		}
	}

	if (auto* Pawn = Cast<APlayerCharacter_SB>(PC->GetPawn()))
	{
		Pawn->DropItemFromSlot(Drag->SourceContainer, Drag->SourceIndex, -1);
	}
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

void UInventoryItemSlot::SetSelectedVisual(bool bSelected)
{
	if (Container != ESlotContainer::Hotbar)
	{
		if (SelectedFrame)
		{
			SelectedFrame->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}
	if (SelectedFrame)
	{
		SelectedFrame->SetVisibility(bSelected ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

