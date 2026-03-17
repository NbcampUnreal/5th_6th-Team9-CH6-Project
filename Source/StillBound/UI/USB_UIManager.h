#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UI/Build/BuildPreview_IngredientPanel.h"
#include "USB_UIManager.generated.h"

class APlayerController;
class UUW_UIHUD;
class UMainMenu;
class UInteractionWidget;
struct FInteractableData;
class UUW_FullMap;
class UUW_RoundProgressBar;
class UHotbarPanel;
class UInventoryComponent;
class UBuildMenuWidget;
class UBuildComponent;
/**
 * 
 */

UENUM(BlueprintType)
enum class EMenuMode : uint8
{
	None,
	InventoryOnly,
	Crafting
};

UCLASS(BlueprintType, Blueprintable)
class STILLBOUND_API USB_UIManager : public UObject
{
	GENERATED_BODY()
	
public:

	void Init(APlayerController* InOwnerPC);

	void SetHP(float Current, float Max);
	void SetStamina(float Current, float Max);
	void SetExp(float Current, float Required);
	void SetLevel(int32 Level);
	void ShowBossHP(const FText& BossName);
	void UpdateBossHP(float Current, float Max);
	void HideBossHP();
	void ShowGatherProgress();
	void HideGatherProgress();
	void UpdateGatherProgress(float Percent);
	void UpdateGatherTime(float Remaining);
	void ToggleFullMap();
	void ShowDamageOverlay();
	void HideDamageOverlay();
	void UpdateDamageOverlay(float HealthPercent);
	void UpdateHUD();

	UUW_FullMap* GetFullMapWidget() const { return FullMapWidget; }
	UUW_UIHUD* GetHUD() const { return UIHUD; }

private:

	UPROPERTY()
	TObjectPtr<APlayerController> OwnerPC;

	UPROPERTY()
	TObjectPtr<UUW_UIHUD> UIHUD;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUW_UIHUD> UIHUDClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUW_FullMap> FullMapClass;

	UPROPERTY()
	TObjectPtr<UUW_FullMap> FullMapWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUW_RoundProgressBar> GatherProgressClass;

	UPROPERTY()
	TObjectPtr<UUW_RoundProgressBar> GatherProgressWidget;

	UPROPERTY()
	class UUW_DamageOverlay* DamageOverlayWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUW_DamageOverlay> DamageOverlayClass;

	UFUNCTION()
	void OnExpChanged(float OldValue, float NewValue);

	UFUNCTION()
	void OnLevelChanged(float OldValue, float NewValue);


public:

	///===============================================================================
	/// PROPERTIES & VARIABLES
	///===============================================================================
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UMainMenu> MainMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UInteractionWidget> InteractionWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UBuildMenuWidget> BuildMenuClass;

	UPROPERTY()
	TObjectPtr<UBuildMenuWidget> BuildMenuWidget;

	///===============================================================================
	/// FUNCTIONS
	///===============================================================================

	void OpenInventoryMenu();
	void OpenCraftingMenu(UInventoryComponent* InInventory);
	void CloseMenu();
	void ToggleMenu();
	void ShowBuildMenu(UBuildComponent* InBuildComponent);
	void HideBuildMenu(UBuildComponent* InBuildComponent);

	bool IsMenuBlockingGameplay() const { return CurrentMenuMode != EMenuMode::None; }

	void ShowInteractionWidget();
	void HideInteractionWidget();
	void UpdateInteractionWidget(const FInteractableData& InteractableData);

	void ShowBuildPreviewPanel();
	void HideBuildPreviewPanel();
	void UpdateBuildPreviewPanel(const TArray<FBuildPreviewCostUIData>& InCosts);
	void ShowBuildPreviewStateMessage(const FText& InMessage, float Duration = 2.f);

	//getter
	TObjectPtr<UMainMenu> GetMainMenuWidget() const { return MainMenuWidget; };

protected:
	///===============================================================================
	/// PROPERTIES & VARIABLES
	///===============================================================================
	UPROPERTY()
	TObjectPtr<UMainMenu> MainMenuWidget;

	UPROPERTY()
	TObjectPtr<UInteractionWidget> InteractionWidget;

	UPROPERTY()
	EMenuMode CurrentMenuMode = EMenuMode::None;

	///===============================================================================
	/// FUNCTIONS
	///===============================================================================
};
