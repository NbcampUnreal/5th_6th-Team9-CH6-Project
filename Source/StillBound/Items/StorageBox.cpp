#include "Items/StorageBox.h"
#include "Inventory/InventoryComponent.h"
#include "Items/ItemBase.h"

AStorageBox::AStorageBox()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	InteractableData.InteractableType = EInteractableType::Container;
	InteractableData.Name = FText::FromString(TEXT("StorageBox"));
	InteractableData.Action = FText::FromString(TEXT("Keep"));
	InteractableData.Quantity = 0;
	InteractableData.InteractionDuration = 0.f;

}

void AStorageBox::BeginPlay()
{
	Super::BeginPlay();

	if (ContainerInventory)
	{
		ContainerInventory->SetUseHotbar(false);
		ContainerInventory->SetSlotsCapacity(60);
		ContainerInventory->SetWeightCapacity(9999.f);
	}
	
}

void AStorageBox::Interact_Implementation(APlayerCharacter_SB* PlayerCharacter)
{
	if (!PlayerCharacter) return;

	if (!ContainerInventory) return;

	/*PlayerCharacter->OpenStorageBoxUI*/

}

FInteractableData AStorageBox::GetInteractableData_Implementation()
{
	return InteractableData;
}

void AStorageBox::BeginFocus_Implementation()
{
	if (Mesh)
	{
		Mesh->SetRenderCustomDepth(true);
	}
}

void AStorageBox::EndFocus_Implementation()
{
	if (Mesh)
	{
		Mesh->SetRenderCustomDepth(false);
	}
}

void AStorageBox::Destroyed()
{
	DropAllStoredItems();
	Super::Destroyed();
}

void AStorageBox::BreakStorageBox()
{
	DropAllStoredItems();
	Destroy();
}

void AStorageBox::DropAllStoredItems()
{
	if (!ContainerInventory) return;

	TArray<UItemBase*> StoredItems;
	ContainerInventory->GetAllItems(StoredItems);

	for (UItemBase* Item : StoredItems)
	{
		if (!Item || Item->Quantity <= 0) continue;

		//드랍 로직

	}

	for (int32 i = 0; i < ContainerInventory->GetInventorySlots().Num(); ++i)
	{
		ContainerInventory->RemoveAmountInContainer(ESlotContainer::Inventory, i, 999999);
	}
}


