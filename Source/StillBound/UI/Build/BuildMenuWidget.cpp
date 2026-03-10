#include "UI/Build/BuildMenuWidget.h"
#include "UI/Build/BuildEntryWidget.h"
#include "Build/BuildComponent.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/WrapBox.h"

void UBuildMenuWidget::Init(UBuildComponent* InBuildComponent)
{
	BuildComponentRef = InBuildComponent;

	if (BuildComponentRef && BuildComponentRef->BuildingDataTable)
	{
		BuildingDataTable = BuildComponentRef->BuildingDataTable;
	}

	RefreshEntries(CurrentCategory);
}

void UBuildMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Structure)
	{
		Btn_Structure->OnClicked.RemoveAll(this);
		Btn_Structure->OnClicked.AddDynamic(this, &ThisClass::OnClickedStructure);
	}

	if (Btn_Production)
	{
		Btn_Production->OnClicked.RemoveAll(this);
		Btn_Production->OnClicked.AddDynamic(this, &ThisClass::OnClickedProduction);
	}
}

void UBuildMenuWidget::OnClickedStructure()
{
	CurrentCategory = EBuildCategory::Structure;
	RefreshEntries(CurrentCategory);
}

void UBuildMenuWidget::OnClickedProduction()
{
	CurrentCategory = EBuildCategory::Production;
	RefreshEntries(CurrentCategory);
}

void UBuildMenuWidget::RefreshEntries(EBuildCategory Category)
{
	if (!EntryWrapBox || !BuildEntryClass || !BuildingDataTable) return;

	EntryWrapBox->ClearChildren();

	static const FString ContextString(TEXT("BuildMenu"));
	TArray<FBuildingDataRow*> AllRows;
	BuildingDataTable->GetAllRows<FBuildingDataRow>(ContextString, AllRows);

	for (const FBuildingDataRow* Row : AllRows)
	{
		if (!Row) continue;
		if (Row->Category != Category) continue;

		UBuildEntryWidget* EntryWidget = CreateWidget<UBuildEntryWidget>(GetOwningPlayer(), BuildEntryClass);
		if (!EntryWidget) continue;

		EntryWidget->InitEntry(Row->BuildingID, Row->Icon, Row->DisplayName);
		EntryWidget->OnBuildEntryClicked.AddUObject(this, &ThisClass::HandleBuildEntryClicked);

		EntryWrapBox->AddChildToWrapBox(EntryWidget);
	}
}

void UBuildMenuWidget::HandleBuildEntryClicked(FName BuildingID)
{
	if (!BuildComponentRef) return;

	BuildComponentRef->BeginBuildMode(BuildingID);

	SetVisibility(ESlateVisibility::Collapsed);
}
