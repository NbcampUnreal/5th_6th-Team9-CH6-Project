#include "UI/Crafting/RecipeEntryWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void URecipeEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (EntryButton)
	{
		EntryButton->OnClicked.RemoveAll(this);
		EntryButton->OnClicked.AddDynamic(this, &URecipeEntryWidget::HandleClicked);
	}
}

void URecipeEntryWidget::InitEntry(FName InRecipeID, UTexture2D* InIcon, const FText& InDisplayName)
{
	RecipeID = InRecipeID;

	if (ResultIcon)
	{
		if (InIcon)
		{
			ResultIcon->SetBrushFromTexture(InIcon, true);
		}
	}

	if (ResultName)
	{
		ResultName->SetText(InDisplayName);
	}
}

void URecipeEntryWidget::HandleClicked()
{
	OnRecipeEntryClicked.Broadcast(RecipeID);
}
