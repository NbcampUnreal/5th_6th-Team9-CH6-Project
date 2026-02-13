#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/InventoryTypes.h"
#include "InventoryItemSlot.generated.h"

class UItemBase;
class UDragItemVisual;
class UInventoryTooltip;
class UBorder;
class UImage;
class UTextBlock;
class UInventoryComponent;

UCLASS()
class STILLBOUND_API UInventoryItemSlot : public UUserWidget
{
	GENERATED_BODY()
	

public:
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	void InitSlot(ESlotContainer InContainer, int32 InIndex, UInventoryComponent* InInv);

	void SetItemReference(UItemBase* ItemIn);

	void DisableDropCatcher();

	FORCEINLINE UItemBase* GetItemReference() const { return ItemReference; };

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Inventory Slot")
	TSubclassOf<UDragItemVisual> DragItemVisualClass;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory Slot")
	TSubclassOf<UInventoryTooltip> ToolTipClass;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory Slot")
	TObjectPtr<UInventoryTooltip> ToolTip;

	UPROPERTY(VisibleAnywhere, Category="Inventory Slot")
	UItemBase* ItemReference;

	UPROPERTY(VisibleAnywhere, Category = "Inventory Slot", meta = (BindWidget))
	UBorder* ItemBorder;

	UPROPERTY(VisibleAnywhere, Category = "Inventory Slot", meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(VisibleAnywhere, Category = "Inventory Slot", meta = (BindWidget))
	UTextBlock* ItemQuantity;

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	UPROPERTY()
	ESlotContainer Container;

	UPROPERTY()
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryRef;
};
