#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "USB_UIManager.generated.h"

class APlayerController;
class UUW_UIHUD;
class UMainMenu;
class UInteractionWidget;
struct FInteractableData;
/**
 * 
 */
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
	void UpdateHUD();
private:

	UPROPERTY()
	TObjectPtr<APlayerController> OwnerPC;

	UPROPERTY()
	TObjectPtr<UUW_UIHUD> UIHUD;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUW_UIHUD> UIHUDClass;

	UFUNCTION()
	void OnExpChanged(float OldValue, float NewValue);

	UFUNCTION()
	void OnLevelChanged(float OldValue, float NewValue);


public:

	//===============================================================================
	// PROPERTIES & VARIABLES
	//===============================================================================
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UMainMenu> MainMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UInteractionWidget> InteractionWidgetClass;

	bool bIsMenuVisible;

	///===============================================================================
	/// FUNCTIONS
	///===============================================================================

	void DisplayMenu();
	void HideMenu();
	void ToggleMenu();

	void ShowInteractionWidget();
	void HideInteractionWidget();
	void UpdateInteractionWidget(const FInteractableData* InteractableData);

protected:
	///===============================================================================
	/// PROPERTIES & VARIABLES
	///===============================================================================
	UPROPERTY()
	TObjectPtr<UMainMenu> MainMenuWidget;

	UPROPERTY()
	TObjectPtr<UInteractionWidget> InteractionWidget;

	///===============================================================================
	/// FUNCTIONS
	///===============================================================================
};
