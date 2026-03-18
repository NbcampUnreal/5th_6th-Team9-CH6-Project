#include "UI/USB_UIManager.h"
#include "UI/UW_UIHUD.h"
#include "UI/UW_Minimap.h"
#include "GameFramework/PlayerController.h"
#include "Character/BaseCharacter_SB.h"
#include "Character/PlayerCharacter_SB.h"
#include "Character/PlayerAttributeSet.h"
#include "UI/MainMenu.h"
#include "UI/UW_FullMap.h"
#include "UI/UW_DamageOverlay.h"
#include "UI/UW_RoundProgressBar.h"
#include "UI/Interaction/InteractionWidget.h"
#include "UI/Inventory/HotbarPanel.h"
#include "UI/Build/BuildMenuWidget.h"

void USB_UIManager::Init(APlayerController* InOwnerPC)
{
	OwnerPC = InOwnerPC;
	if (!OwnerPC || !UIHUDClass) return;

	UIHUD = CreateWidget<UUW_UIHUD>(OwnerPC, UIHUDClass);
	if (!UIHUD) return;

	UIHUD->AddToViewport(10);

	if (MainMenuClass)
	{
		MainMenuWidget = CreateWidget<UMainMenu>(OwnerPC, MainMenuClass);
		if (!MainMenuWidget) return;

		MainMenuWidget->AddToViewport(15);
		MainMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (InteractionWidgetClass)
	{
		InteractionWidget = CreateWidget<UInteractionWidget>(OwnerPC, InteractionWidgetClass);
		if (!InteractionWidget) return;

		InteractionWidget->AddToViewport(5);
		InteractionWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (FullMapClass)
	{
		FullMapWidget = CreateWidget<UUW_FullMap>(OwnerPC, FullMapClass);
	}

	if (DamageOverlayClass)
	{
		DamageOverlayWidget = CreateWidget<UUW_DamageOverlay>(OwnerPC, DamageOverlayClass);

		if (DamageOverlayWidget)
		{
			DamageOverlayWidget->AddToViewport();
			DamageOverlayWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (GatherProgressClass)
	{
		GatherProgressWidget = CreateWidget<UUW_RoundProgressBar>(OwnerPC, GatherProgressClass);

		if (GatherProgressWidget)
		{
			GatherProgressWidget->AddToViewport(6);
			GatherProgressWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (BuildMenuClass)
	{
		BuildMenuWidget = CreateWidget<UBuildMenuWidget>(OwnerPC, BuildMenuClass);
		if (BuildMenuWidget)
		{
			BuildMenuWidget->AddToViewport(20);
			BuildMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
	if (PauseMenuClass)
	{
		PauseMenuWidget = CreateWidget<UUserWidget>(OwnerPC, PauseMenuClass);
		if (PauseMenuWidget)
		{
			PauseMenuWidget->AddToViewport(100);
			PauseMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	ABaseCharacter_SB* Char = Cast<ABaseCharacter_SB>(OwnerPC->GetPawn());
	if (!Char) return;

	UPlayerAttributeSet* AS = Char->GetPlayerAttributeSet();
	if (!AS) return;

	AS->OnExperienceChanged.AddDynamic(this, &USB_UIManager::OnExpChanged);
	AS->OnLevelChanged.AddDynamic(this, &USB_UIManager::OnLevelChanged);

	UpdateHUD();
	
	if (APlayerCharacter_SB* Chr = Cast<APlayerCharacter_SB>(Char))
	{
		UIHUD->InitInventory(Chr->GetInventory());
	}
}

void USB_UIManager::SetHP(float Current, float Max)
{
	if (!UIHUD) return;

	UIHUD->SetHP(Current, Max);
}

void USB_UIManager::SetStamina(float Current, float Max)
{
	if (!UIHUD) return;
	UIHUD->SetStamina(Current, Max); 
}

void USB_UIManager::SetExp(float Current, float Required)
{
	if (!UIHUD) return;
	UIHUD->SetExp(Current, Required);
}

void USB_UIManager::ShowBossHP(const FText& BossName)
{
	if (!UIHUD) return;

	UIHUD->SetBossName(BossName);
}

void USB_UIManager::UpdateBossHP(float Current, float Max)
{
	if (!UIHUD) return;

	UIHUD->SetBossHP(Current, Max);
}

void USB_UIManager::HideBossHP()
{
	if (!UIHUD) return;

	UIHUD->HideBossHP();
}

//void USB_UIManager::SetLevel(int32 Level)
//{
//	if (!UIHUD) return;
//	UIHUD->SetLevel(Level);
//}

void USB_UIManager::ShowGatherProgress()
{
	if (GatherProgressWidget)
	{
		GatherProgressWidget->SetPercent(0.f);

		GatherProgressWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void USB_UIManager::HideGatherProgress()
{
	if (GatherProgressWidget)
	{
		GatherProgressWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USB_UIManager::UpdateGatherProgress(float Percent)
{
	if (GatherProgressWidget)
	{
		GatherProgressWidget->SetPercent(Percent);
	}
}

void USB_UIManager::UpdateGatherTime(float Remaining)
{
	if (GatherProgressWidget)
	{
		GatherProgressWidget->SetRemainingTime(Remaining);
	}
}

void USB_UIManager::ToggleFullMap()
{
	if (!FullMapWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("FullMapWidget is NULL"));
		return;
	}

	if (FullMapWidget->IsInViewport())
	{
		UE_LOG(LogTemp, Warning, TEXT("Removing FullMap"));
		FullMapWidget->RemoveFromParent();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Adding FullMap"));
		FullMapWidget->AddToViewport(50);
	}
}

void USB_UIManager::ShowDamageOverlay()
{
	if (DamageOverlayWidget)
	{
		DamageOverlayWidget->SetVisibility(ESlateVisibility::Visible);
	}
}


void USB_UIManager::HideDamageOverlay()
{
	if (DamageOverlayWidget)
	{
		DamageOverlayWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void USB_UIManager::UpdateDamageOverlay(float HealthPercent)
{
	if (DamageOverlayWidget)
	{
		DamageOverlayWidget->UpdateDamageEffect(HealthPercent);
	}
}

void USB_UIManager::UpdateHUD()
{
	if (!OwnerPC || !UIHUD) return;

	ABaseCharacter_SB* Char = Cast<ABaseCharacter_SB>(OwnerPC->GetPawn());
	if (!Char) return;

	UPlayerAttributeSet* AS = Char->GetPlayerAttributeSet();
	if (!AS) return;

	UIHUD->SetHP(AS->GetHealth(), AS->GetMaxHealth());
	UIHUD->SetStamina(AS->GetStamina(), AS->GetMaxStamina());

	const float CurExp = AS->GetExperience();
	const int32 Level = AS->GetLevel();
	const float RequiredExp = AS->GetRequiredExpForLevel(Level);

	UIHUD->SetExp(CurExp, RequiredExp);
	UIHUD->SetLevel(Level);
}

void USB_UIManager::OnExpChanged(float OldValue, float NewValue)
{
	UpdateHUD();
}

void USB_UIManager::OnLevelChanged(float OldValue, float NewValue)
{
	UpdateHUD();
}

void USB_UIManager::OpenInventoryMenu()
{
	if (!OwnerPC || !MainMenuWidget) return;

	MainMenuWidget->ShowInventoryOnly();

	CurrentMenuMode = EMenuMode::InventoryOnly;

	MainMenuWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void USB_UIManager::OpenCraftingMenu(UInventoryComponent* InInventory)
{
	if (!OwnerPC || !MainMenuWidget || !InInventory) return;

	MainMenuWidget->ShowCrafting(InInventory);

	CurrentMenuMode = EMenuMode::Crafting;

	MainMenuWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void USB_UIManager::CloseMenu()
{
	if (!OwnerPC || !MainMenuWidget) return;

	CurrentMenuMode = EMenuMode::None;

	MainMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void USB_UIManager::ToggleMenu()
{
	if (CurrentMenuMode == EMenuMode::None)
	{
		OpenInventoryMenu();
	}
	else
	{
		CloseMenu();
	}

}

void USB_UIManager::ShowBuildMenu(UBuildComponent* InBuildComponent)
{

	if (BuildMenuWidget)
	{
		BuildMenuWidget->SetVisibility(ESlateVisibility::Visible);
		BuildMenuWidget->Init(InBuildComponent);
	}

}

void USB_UIManager::HideBuildMenu(UBuildComponent* InBuildComponent)
{
	if (BuildMenuWidget)
	{
		BuildMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USB_UIManager::ShowInteractionWidget()
{
	if (InteractionWidget)
	{
		InteractionWidget->SetVisibility(ESlateVisibility::Visible);
	}
}
void USB_UIManager::HideInteractionWidget()
{
	if (InteractionWidget)
	{
		InteractionWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USB_UIManager::UpdateInteractionWidget(const FInteractableData& InteractableData)
{
	if (InteractionWidget)
	{
		if (InteractionWidget->GetVisibility() == ESlateVisibility::Collapsed)
		{
			InteractionWidget->SetVisibility(ESlateVisibility::Visible);
		}

		InteractionWidget->UpdateWidget(InteractableData);
	}
}

void USB_UIManager::ShowBuildPreviewPanel()
{
	if (!UIHUD) return;

	if (UBuildPreview_IngredientPanel* Panel = UIHUD->GetBuildPreview_IngredientPanel())
	{
		Panel->SetVisibility(ESlateVisibility::Visible);
	}
}

void USB_UIManager::HideBuildPreviewPanel()
{
	if (!UIHUD) return;

	if (UBuildPreview_IngredientPanel* Panel = UIHUD->GetBuildPreview_IngredientPanel())
	{
		Panel->SetVisibility(ESlateVisibility::Collapsed);
		Panel->HidePlacementStateMessage();
	}
}

void USB_UIManager::UpdateBuildPreviewPanel(const TArray<FBuildPreviewCostUIData>& InCosts)
{
	if (!UIHUD) return;

	if (UBuildPreview_IngredientPanel* Panel = UIHUD->GetBuildPreview_IngredientPanel())
	{
		Panel->UpdateIngredientList(InCosts);
	}
}

void USB_UIManager::ShowBuildPreviewStateMessage(const FText& InMessage, float Duration)
{
	if (!UIHUD) return;

	if (UBuildPreview_IngredientPanel* Panel = UIHUD->GetBuildPreview_IngredientPanel())
	{
		Panel->ShowPlacementStateMessage(InMessage, Duration);
	}
}

bool USB_UIManager::IsPauseMenuOpen() const
{
	return PauseMenuWidget && PauseMenuWidget->GetVisibility() != ESlateVisibility::Collapsed;
}

void USB_UIManager::OpenPauseMenu()
{
	if (!PauseMenuWidget) return;
	PauseMenuWidget->SetVisibility(ESlateVisibility::Visible);
}

void USB_UIManager::ClosePauseMenu()
{
	if (!PauseMenuWidget) return;
	PauseMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
}
