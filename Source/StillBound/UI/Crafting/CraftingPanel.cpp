#include "UI/Crafting/CraftingPanel.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"
#include "Inventory/InventoryComponent.h"
#include "Public/Data/ItemData.h"
#include "UI/Crafting/IngredientRowWidget.h"
#include "UI/Crafting/RecipeEntryWidget.h"

void UCraftingPanel::Init(UInventoryComponent* InInv)
{
	Inv = InInv;
	if (!Inv) return;

	if (!bBoundInventoryEvent)
	{
		Inv->OnInventoryUpdated.AddUniqueDynamic(this, &UCraftingPanel::RefreshDetail);
		bBoundInventoryEvent = true;
	}

	RebuildRecipeList();

	// 처음 선택 레시피 없으면 첫 번째 레시피 자동 선택
	if (SelectedRecipeID.IsNone() && Inv->RecipeDataTable)
	{
		const TArray<FName> RecipeIDs = Inv->RecipeDataTable->GetRowNames();
		if (RecipeIDs.Num() > 0)
		{
			SelectedRecipeID = RecipeIDs[0];
		}
	}

	RefreshDetail();
}

void UCraftingPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (CraftButton)
	{
		CraftButton->OnClicked.RemoveAll(this);
		CraftButton->OnClicked.AddDynamic(this, &UCraftingPanel::OnCraftClicked);
	}
}

void UCraftingPanel::OnCraftClicked()
{
	if (!Inv || SelectedRecipeID.IsNone()) return;

	const FCraftResult R = Inv->Craft(SelectedRecipeID, 1);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.f,
			R.bSuccess ? FColor::Green : FColor::Red,
			R.Message.ToString()
		);
	}

	RebuildRecipeList();
	RefreshDetail();
}

void UCraftingPanel::RebuildRecipeList()
{
	if (!Inv || !Inv->RecipeDataTable || !RecipeScroll || !RecipeEntryClass) return;

	if (RecipeWrapBox)
	{
		RecipeWrapBox->ClearChildren();
	}
	else if (RecipeScroll)
	{
		RecipeScroll->ClearChildren();
	}
	else
	{
		return;
	}

	const TArray<FName> RecipeIDs = Inv->RecipeDataTable->GetRowNames();

	for (const FName& RecipeID : RecipeIDs)
	{
		FCraftingRecipeRow Row;
		if (!Inv->GetRecipeRowForUI(RecipeID, Row)) continue;

		URecipeEntryWidget* EntryWidget = CreateWidget<URecipeEntryWidget>(GetOwningPlayer(), RecipeEntryClass);
		if (!EntryWidget) continue;

		UTexture2D* IconTex = FindItemIconByID(Row.ResultItemID);
		const FText DisplayName = FindItemDisplayNameByID(Row.ResultItemID);

		EntryWidget->InitEntry(RecipeID, IconTex, DisplayName);
		EntryWidget->OnRecipeEntryClicked.AddUObject(this, &UCraftingPanel::SelectRecipe);

		if (RecipeWrapBox)
		{
			RecipeWrapBox->AddChildToWrapBox(EntryWidget);
		}
		else if (RecipeScroll)
		{
			RecipeScroll->AddChild(EntryWidget);
		}
	}
}

void UCraftingPanel::RefreshDetail()
{
	if (!Inv) return;

	if (SelectedRecipeID.IsNone())
	{
		if (IngredientBox)
		{
			IngredientBox->ClearChildren();
		}

		if (ResultText)
		{
			ResultText->SetText(FText::FromString(TEXT("레시피 선택")));
		}

		if (CraftButton)
		{
			CraftButton->SetIsEnabled(false);
		}

		return;
	}

	FCraftingRecipeRow Row;
	if (!Inv->GetRecipeRowForUI(SelectedRecipeID, Row))
	{
		if (IngredientBox)
		{
			IngredientBox->ClearChildren();
		}

		if (ResultText)
		{
			ResultText->SetText(FText::FromString(TEXT("레시피 없음")));
		}

		if (CraftButton)
		{
			CraftButton->SetIsEnabled(false);
		}

		return;
	}

	// 아래 한 줄 결과 텍스트만 간단히 표시
	if (ResultText)
	{
		ResultText->SetText(FText::Format(
			FText::FromString(TEXT("{0} x{1}")),
			FindItemDisplayNameByID(Row.ResultItemID),
			FText::AsNumber(Row.ResultCount)
		));
	}

	if (IngredientBox)
	{
		IngredientBox->ClearChildren();
	}

	TArray<FCraftMissing> Missing;
	const bool bCanCraft = Inv->CanCraft(SelectedRecipeID, 1, Missing);

	// 오른쪽 재료 목록 채우기
	for (const FRecipeIngredient& Ing : Row.Ingredients)
	{
		const int32 Have = Inv->GetTotalCountByID_ForUI(Ing.ItemID);
		const int32 Need = Ing.Count;

		const FText DisplayName = FindItemDisplayNameByID(Ing.ItemID);
		UTexture2D* IconTex = FindItemIconByID(Ing.ItemID);

		if (IngredientRowClass)
		{
			UIngredientRowWidget* RowWidget =
				CreateWidget<UIngredientRowWidget>(GetOwningPlayer(), IngredientRowClass);

			if (RowWidget)
			{
				RowWidget->InitRow(DisplayName, Have, Need, IconTex);
				IngredientBox->AddChild(RowWidget);
				continue;
			}
		}

		// 혹시 IngredientRowClass 없을 때의 fallback
		UTextBlock* FallbackText = NewObject<UTextBlock>(this);
		if (FallbackText)
		{
			FallbackText->SetText(FText::Format(
				FText::FromString(TEXT("{0} : {1}/{2}")),
				DisplayName,
				FText::AsNumber(Have),
				FText::AsNumber(Need)
			));

			IngredientBox->AddChild(FallbackText);
		}
	}

	if (CraftButton)
	{
		CraftButton->SetIsEnabled(bCanCraft);
	}
}

void UCraftingPanel::SelectRecipe(FName RecipeID)
{
	SelectedRecipeID = RecipeID;
	RefreshDetail();
}

UTexture2D* UCraftingPanel::FindItemIconByID(FName ItemID) const
{
	if (!Inv || !Inv->ItemDataTable) return nullptr;

	const FItemDataRow* ItemRow = Inv->ItemDataTable->FindRow<FItemDataRow>(ItemID, TEXT("CraftingUI"));
	if (!ItemRow) return nullptr;

	return ItemRow->AssetData.Icon;
}

FText UCraftingPanel::FindItemDisplayNameByID(FName ItemID) const
{
	if (!Inv || !Inv->ItemDataTable) return FText::FromName(ItemID);

	const FItemDataRow* ItemRow = Inv->ItemDataTable->FindRow<FItemDataRow>(ItemID, TEXT("CraftingUI"));
	if (!ItemRow) return FText::FromName(ItemID);

	return ItemRow->TextData.Name;
}
