#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Data/InventoryTypes.h"
#include "ItemDragDropOperation.generated.h"

class UItemBase;
class UInventoryComponent;

UCLASS()
class STILLBOUND_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UItemBase* SourceItem;

	UPROPERTY()
	UInventoryComponent* SourceInventory;

	UPROPERTY()
	ESlotContainer SourceContainer;

	UPROPERTY()
	int32 SourceIndex = INDEX_NONE;
	
};
