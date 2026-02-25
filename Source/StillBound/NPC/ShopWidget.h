// ShopWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopWidget.generated.h"

class ANPCCharacter;
class UInventoryComponent;
class UTextBlock;
class UButton;
class UWrapBox;
class UBorder;

UCLASS()
class STILLBOUND_API UShopWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

public:
    UFUNCTION(BlueprintCallable, Category = "Shop")
    void InitializeShop(ANPCCharacter* NPC);

    UFUNCTION(BlueprintCallable, Category = "Shop")
    void CloseShop();

    UFUNCTION(BlueprintCallable, Category = "Shop")
    void RefreshShop();

protected:
    // ========== 바인드 위젯 ==========

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ShopName;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_PlayerGold;

    UPROPERTY(meta = (BindWidget))
    UWrapBox* WB_ShopItems;

    UPROPERTY(meta = (BindWidget))
    UWrapBox* WB_PlayerInventory;

    UPROPERTY(meta = (BindWidget))
    UButton* BTN_Close;

    // 탭 전환 (선택사항)
    UPROPERTY(meta = (BindWidget))
    UButton* BTN_BuyTab;

    UPROPERTY(meta = (BindWidget))
    UButton* BTN_SellTab;

    UPROPERTY(meta = (BindWidget))
    UBorder* Border_BuyPanel;

    UPROPERTY(meta = (BindWidget))
    UBorder* Border_SellPanel;

    // ========== 설정 ==========

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    TSubclassOf<class UShopItemSlot> ShopItemSlotClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    TSubclassOf<class UInventoryItemSlot> InventoryItemSlotClass;

private:
    UPROPERTY()
    ANPCCharacter* NPCCharacter = nullptr;

    UPROPERTY()
    UInventoryComponent* PlayerInventory = nullptr;

    bool bShowBuyTab = true;

    // 버튼 콜백
    UFUNCTION()
    void OnCloseButtonClicked();

    UFUNCTION()
    void OnBuyTabClicked();

    UFUNCTION()
    void OnSellTabClicked();

    // UI 업데이트
    void DisplayShopItems();
    void DisplayPlayerInventory();
    void UpdateGoldDisplay();
    void SwitchTab(bool bBuyTab);
};