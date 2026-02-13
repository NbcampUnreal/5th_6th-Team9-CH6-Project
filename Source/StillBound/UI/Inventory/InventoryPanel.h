#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryPanel.generated.h"

class UUniformGridPanel;
class UUniformGridSlot;
class UTextBlock;
class APlayerCharacter_SB;
class UInventoryComponent;
class UInvnetoryItemSlot;
class UInventoryItemSlot;


UCLASS()
class STILLBOUND_API UInventoryPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void RefreshInventory();

	UPROPERTY(meta=(BindWidget))
	UUniformGridPanel* InventoryGrid;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeightInfo;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CapacityInfo;

	UPROPERTY()
	APlayerCharacter_SB* PlayerCharacter;

	UPROPERTY()
	UInventoryComponent* InventoryReference;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventoryItemSlot> InventorySlotClass;

	UPROPERTY()
	TArray<TObjectPtr<UInventoryItemSlot>> SlotWidgets;

protected:
	virtual void NativeOnInitialized() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	void SetInfoText() const;

	void BuildSlotGrid();

private:
	static constexpr int32 Cols = 6;
	static constexpr int32 Rows = 8;
	static constexpr int32 MaxSlots = Cols * Rows;

};
