#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Data/InventoryTypes.h"
#include "ItemDragDropOperation.generated.h"

class UItemBase;

UCLASS()
class STILLBOUND_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UItemBase> SourceItem = nullptr;

	UPROPERTY()
	ESlotContainer SourceContainer = ESlotContainer::Inventory;

	UPROPERTY()
	int32 SourceIndex = INDEX_NONE;
	
};
