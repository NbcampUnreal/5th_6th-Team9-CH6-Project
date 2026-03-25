#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_UIHUD.generated.h"

class UUW_HPBar;
class UUW_StaminaBar;
class UUW_ExpBar;
class UUW_Minimap;
class UBossHPbar;
class UVerticalBox;
class UUW_PickupText;
class UBorder;
class UHotbarPanel;
class UInventoryComponent;
class UBuildPreview_IngredientPanel;
class UImage;
class UUW_Crosshair;

/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_UIHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetHP(float Current, float Max);

	UFUNCTION(BlueprintCallable)
	void SetStamina(float Current, float Max);

	UFUNCTION(BlueprintCallable)
	void SetExp(float Current, float Required);

	UFUNCTION(BlueprintCallable)
	void SetLevel(int32 Level);

	void AddPickupLog(const FText& Text);
	void CheckPickupPanel();
	void SetBossName(const FText& Name);
	void SetBossHP(float Current, float Max);
	void HideBossHP();

	UUW_Minimap* GetMiniMapWidget() const { return MiniMapWidget; }

	void InitInventory(UInventoryComponent* InInv);

	TObjectPtr<UHotbarPanel> GetHotbarPanel() const { return HotbarPanel; };

	void SetSelectedHotbarIndex(int32 Index);

	UBuildPreview_IngredientPanel* GetBuildPreview_IngredientPanel() const { return BuildPreview_IngredientPanel; }

	void SetBuildGuideVisibile(bool bVisible);

	void SetCrosshairVisible(bool bVisible);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_HPBar> HPBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_StaminaBar> StaminaBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_ExpBar> ExpBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_Minimap> MiniMapWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> PickupPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PickupLogBox;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	TSubclassOf<UUW_PickupText> PickupTextClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBossHPbar> BossHP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHotbarPanel> HotbarPanel;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBuildPreview_IngredientPanel> BuildPreview_IngredientPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BuildGuideImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_Crosshair> Crosshair;

};
