#include "UI/MainMenu.h"
#include "Character/PlayerCharacter_SB.h"
#include "Inventory/ItemDragDropOperation.h"
#include "Components/Border.h"
#include "Items/ItemBase.h"

void UMainMenu::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UMainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerCharacter = Cast<APlayerCharacter_SB>(GetOwningPlayerPawn());
	
	EnableDropCatcher(false);
}

bool UMainMenu::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const UItemDragDropOperation* ItemDragDrop = Cast<UItemDragDropOperation>(InOperation);

	if (!PlayerCharacter || !ItemDragDrop)
	{
		EnableDropCatcher(false);
		return false;
	}

	if (ItemDragDrop->SourceIndex != INDEX_NONE)
	{
		PlayerCharacter->DropItemFromSlot(ItemDragDrop->SourceContainer, ItemDragDrop->SourceIndex, -1);

		EnableDropCatcher(false);
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("[MainMenuDrop]"));

	EnableDropCatcher(false);
	return false;


}

void UMainMenu::EnableDropCatcher(bool bEnable)
{
	if (!DropCatcher) return;

	/*DropCatcher->SetVisibility(bEnable ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);*/
	DropCatcher->SetVisibility(ESlateVisibility::Collapsed);
}
