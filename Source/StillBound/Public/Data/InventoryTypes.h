#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.generated.h"

UENUM(BlueprintType)
enum class ESlotContainer : uint8
{
	Inventory UMETA(DisplayName = "Inventory"),
	Hotbar UMETA(DisplayName = "Hotbar")
};
