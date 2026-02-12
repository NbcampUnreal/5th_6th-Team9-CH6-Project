#include "UI/Inventory/HotbarPanel.h"
#include "Character/PlayerCharacter_SB.h"
#include "Inventory/InventoryComponent.h"
#include "UI/Inventory/InventoryItemSlot.h"
#include "Components/UniformGridPanel.h"


void UHotbarPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PlayerCharacter = Cast<APlayerCharacter_SB>(GetOwningPlayerPawn());
	if (!PlayerCharacter) return;

	InventoryReference = PlayerCharacter->GetInventory();
	if (!InventoryReference) return;

	InventoryReference->OnHotbarUpdated.AddUObject(this, &UHotbarPanel::RefreshHotbar);

	BuildHotbar();
	RefreshHotbar();


}

void UHotbarPanel::BuildHotbar()
{
	if (!HotbarGrid || !HotbarSlotClass) return;
	if (HotbarSlotWidgets.Num() > 0) return;

	HotbarGrid->ClearChildren();
	HotbarSlotWidgets.Reserve(HotbarSize);

	for (int32 Index = 0; Index < HotbarSize; ++Index)
	{
		UInventoryItemSlot* SlotWidget = CreateWidget<UInventoryItemSlot>(this, HotbarSlotClass);
		if (!SlotWidget) continue;

		SlotWidget->InitSlot(ESlotContainer::Hotbar, Index, InventoryReference);
		SlotWidget->SetItemReference(nullptr);

		HotbarGrid->AddChildToUniformGrid(SlotWidget, 0, Index);

		HotbarSlotWidgets.Add(SlotWidget);
	}
}

void UHotbarPanel::RefreshHotbar()
{
	if (!InventoryReference) return;
	if (HotbarSlotWidgets.Num() == 0) return;

	const TArray<UItemBase*>& Hotbar = InventoryReference->GetHatbarContents();

	for (int32 i = 0; i < HotbarSize; ++i)
	{
		UInventoryItemSlot* SlotWidget = HotbarSlotWidgets[i];
		if (!SlotWidget) continue;

		UItemBase* Item = Hotbar.IsValidIndex(i) ? Hotbar[i] : nullptr;
		SlotWidget->SetItemReference(Item);
	}
}

