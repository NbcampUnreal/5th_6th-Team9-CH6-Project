// ShopWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopWidget.generated.h"

// Forward Declarations
class ANPCCharacter;
class UInventoryComponent;
class UShopItemSlot;
class UInventoryItemSlot;
class UWrapBox;
class UBorder;
class UTextBlock;
class UButton;

UCLASS()
class STILLBOUND_API UShopWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // ===== 드래그 앤 드롭 =====
    virtual bool NativeOnDrop(
        const FGeometry& InGeometry,
        const FDragDropEvent& InDragDropEvent,
        UDragDropOperation* InOperation) override;

    virtual void NativeOnDragEnter(
        const FGeometry& InGeometry,
        const FDragDropEvent& InDragDropEvent,
        UDragDropOperation* InOperation) override;

    virtual void NativeOnDragLeave(
        const FDragDropEvent& InDragDropEvent,
        UDragDropOperation* InOperation) override;

public:
    // ===== Public 함수 =====
    void InitializeShop(ANPCCharacter* NPC);
    bool BuyItem(FName ItemID, int32 Quantity);

private:
    // ===== Button Callbacks =====
    UFUNCTION()
    void OnCloseButtonClicked();

    UFUNCTION()
    void OnBuyTabClicked();

    UFUNCTION()
    void OnSellTabClicked();

    // ===== Internal 함수 =====
    void RefreshShop();
    void DisplayShopItems();
    void DisplayPlayerInventory();
    void UpdateGoldDisplay();
    void SwitchTab(bool bBuyTab);
    void CloseShop();

    UFUNCTION()
    void OnRestockTimer();

    // ===== References =====
    UPROPERTY()
    ANPCCharacter* NPCCharacter;

    UPROPERTY()
    UInventoryComponent* PlayerInventory;

    // ===== UI Widgets - Left Panel (Player Inventory) =====
    UPROPERTY(meta = (BindWidget))
    UBorder* Border_LeftPanel;

    UPROPERTY(meta = (BindWidget))
    UWrapBox* WB_PlayerInventory;

    // ===== UI Widgets - Right Panel (Shop) =====
    UPROPERTY(meta = (BindWidget))
    UBorder* Border_RightPanel;

    UPROPERTY(meta = (BindWidget))
    UBorder* Border_BuyPanel;

    UPROPERTY(meta = (BindWidget))
    UWrapBox* WB_ShopItems;

    UPROPERTY(meta = (BindWidget))
    UBorder* Border_SellPanel;

    UPROPERTY(meta = (BindWidget))
    UBorder* DropZone;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_DropHint;

    // ===== UI Widgets - Common =====
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ShopName;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_PlayerGold;

    UPROPERTY(meta = (BindWidget))
    UButton* BTN_Close;

    UPROPERTY(meta = (BindWidget))
    UButton* BTN_BuyTab;

    UPROPERTY(meta = (BindWidget))
    UButton* BTN_SellTab;

    // ===== Widget Classes =====
    UPROPERTY(EditDefaultsOnly, Category = "Shop")
    TSubclassOf<UShopItemSlot> ShopItemSlotClass;

    UPROPERTY(EditDefaultsOnly, Category = "Shop")
    TSubclassOf<UInventoryItemSlot> InventoryItemSlotClass;

    // ===== State =====
    bool bShowBuyTab = true;

    // ===== Timer =====
    FTimerHandle RestockTimerHandle;

    UPROPERTY(EditDefaultsOnly, Category = "Shop")
    float RestockIntervalSeconds = 600.0f;  // 10분
};