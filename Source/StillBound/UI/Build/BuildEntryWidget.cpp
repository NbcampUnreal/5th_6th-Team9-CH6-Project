#include "UI/Build/BuildEntryWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UBuildEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (EntryButton)
	{
		EntryButton->OnClicked.RemoveAll(this);
		EntryButton->OnClicked.AddDynamic(this, &ThisClass::HandleClicked);
	}
}

void UBuildEntryWidget::InitEntry(FName InBuildingID, UTexture2D* InIcon, const FText& InDisPlayName)
{
	BuildingID = InBuildingID;

	if (BuildingIcon)
	{
		if (InIcon)
		{
			BuildingIcon->SetBrushFromTexture(InIcon, true);
		}
		else
		{
			BuildingIcon->SetBrushFromTexture(nullptr);
		}
	}

	if (BuildingNameText)
	{
		BuildingNameText->SetText(InDisPlayName);
	}
}

void UBuildEntryWidget::HandleClicked()
{
	OnBuildEntryClicked.Broadcast(BuildingID);
}