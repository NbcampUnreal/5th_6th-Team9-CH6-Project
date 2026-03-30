#include "UI/UW_UIHUD.h"
#include "UI/UW_HPBar.h"
#include "UI/UW_StaminaBar.h"
#include "UI/UW_ExpBar.h"
#include "UI/UW_Minimap.h"
#include "UI/UW_BossHPbar.h"
#include "UI/UW_PickupText.h"
#include "UI/UW_Crosshair.h"
#include "Components/Border.h"
#include "UI/Inventory/HotbarPanel.h"
#include "Components/VerticalBox.h"
#include "Inventory/InventoryComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

void UUW_UIHUD::NativeConstruct()
{
	Super::NativeConstruct();

	SetBuildGuideVisibile(false);
}

void UUW_UIHUD::SetHP(float Current, float Max)
{
	if (!HPBar) return;

	HPBar->SetHP(Current, Max);
}

void UUW_UIHUD::SetStamina(float Current, float Max)
{
	//UE_LOG(LogTemp, Warning, TEXT("[UIHUD] SetStamin: %f / %f"), Current, Max);

	if (StaminaBar)
	{
		StaminaBar->SetStamina(Current, Max);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[UIHUD] StaminaBar is null (BindWidget failed?)"));
	}
}

void UUW_UIHUD::SetExp(float Current, float Required)
{
	if (ExpBar)
	{
		ExpBar->SetExp(Current, Required);
	}
}

void UUW_UIHUD::SetLevel(int32 Level)
{
	if (ExpBar)
	{
		ExpBar->SetLevel(Level);
	}
}

void UUW_UIHUD::AddPickupLog(const FText& Text)
{
	if (!PickupLogBox || !PickupTextClass)
	{
		return;
	}

	FString RawText = Text.ToString();

	FString ItemName;
	int32 Amount = 1;

	int32 PlusIndex;
	if (RawText.FindLastChar(TEXT('+'), PlusIndex))
	{
		ItemName = RawText.Left(PlusIndex).TrimEnd();

		FString AmountString = RawText.Mid(PlusIndex + 1).TrimStartAndEnd();

		if (AmountString.IsNumeric())
		{
			Amount = FCString::Atoi(*AmountString);
		}
	}
	else
	{
		ItemName = RawText;
	}

	for (int32 i = 0; i < PickupLogBox->GetChildrenCount(); i++)
	{
		UUW_PickupText* Existing =
			Cast<UUW_PickupText>(PickupLogBox->GetChildAt(i));

		if (Existing && Existing->GetBaseText() == ItemName)
		{
			Existing->AddStack(Amount);
			Existing->StartLifeTimer(2.f);
			return;
		}
	}

	UUW_PickupText* PickupWidget =
		CreateWidget<UUW_PickupText>(GetOwningPlayer(), PickupTextClass);

	if (!PickupWidget)
	{
		return;
	}

	PickupPanel->SetVisibility(ESlateVisibility::Visible);

	PickupWidget->SetPickupText(FText::FromString(ItemName), Amount);
	PickupWidget->StartLifeTimer(2.f);

	PickupLogBox->InsertChildAt(0, PickupWidget);
}

void UUW_UIHUD::CheckPickupPanel()
{
	UE_LOG(LogTemp, Warning, TEXT("CheckPickupPanel Called"));

	if (!PickupLogBox || !PickupPanel)
	{
		UE_LOG(LogTemp, Warning, TEXT("PickupLogBox or PickupPanel NULL"));
		return;
	}

	int32 Count = PickupLogBox->GetChildrenCount();

	UE_LOG(LogTemp, Warning, TEXT("Pickup Children Count: %d"), Count);

	if (Count == 0)
	{
		PickupPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UUW_UIHUD::SetBossName(const FText& Name)
{
	if (!BossHP) return;

	BossHP->SetVisibility(ESlateVisibility::Visible);
	BossHP->SetBossName(Name);
}

void UUW_UIHUD::SetBossHP(float Current, float Max)
{
	if (!BossHP) return;

	BossHP->SetHPPercent(Current / Max);
}

void UUW_UIHUD::HideBossHP()
{
	if (!BossHP) return;

	BossHP->SetVisibility(ESlateVisibility::Hidden);
}

void UUW_UIHUD::InitInventory(UInventoryComponent* InInv)
{
	if (HotbarPanel)
	{
		HotbarPanel->InitWithInventory(InInv);
	}
}

void UUW_UIHUD::SetSelectedHotbarIndex(int32 Index)
{
	if (HotbarPanel)
	{
		HotbarPanel->SetSelectedIndex(Index);
	}
}

void UUW_UIHUD::SetBuildGuideVisibile(bool bVisible)
{
	if (BuildGuideImage)
	{
		BuildGuideImage->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UUW_UIHUD::SetCrosshairVisible(bool bVisible)
{
	if (Crosshair)
	{
		Crosshair->SetVisibility(
			bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden
		);
	}
}

//====채집 불가 메시지
void UUW_UIHUD::ShowGatherFailMessage(const FText& Message)
{
	// 텍스트 설정
	if (GatherFailText)
	{
		GatherFailText->SetText(Message);
	}

	// 패널 표시
	if (GatherFailPanel)
	{
		GatherFailPanel->SetVisibility(ESlateVisibility::Visible);
	}

	//2초 후 자동 숨김
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->GetWorldTimerManager().ClearTimer(GatherFailTimerHandle);
		PC->GetWorldTimerManager().SetTimer(
			GatherFailTimerHandle,
			this,
			&UUW_UIHUD::HideGatherFailMessage,
			2.0f,
			false
		);
	}
}

void UUW_UIHUD::HideGatherFailMessage()
{
	if (GatherFailPanel)
	{
		GatherFailPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}