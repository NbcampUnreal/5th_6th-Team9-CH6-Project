// ShopWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopWidget.generated.h"

class ANPCCharacter;
class APlayerCharacter_SB;
class UInventoryComponent;
class UShopItemSlot;
class UInventoryItemSlot;
class UWrapBox;
class UBorder;
class UButton;
class UTextBlock;
class UScrollBox;
class UDataTable;

USTRUCT()
struct FSoldItemRecord
{
    GENERATED_BODY()

    UPROPERTY()
    FName ItemRowName = NAME_None;

    UPROPERTY()
    int32 Quantity = 0;

    UPROPERTY()
    int32 GoldEarned = 0;
};

UCLASS()
class STILLBOUND_API UShopWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // === 초기화 ===
    UFUNCTION(BlueprintCallable, Category = "Shop")
    void InitializeShop(ANPCCharacter* InNPCCharacter);

    // === 상점 새로고침 ===
    UFUNCTION(BlueprintCallable, Category = "Shop")
    void RefreshShop();

    // === 아이템 구매 ===
    UFUNCTION(BlueprintCallable, Category = "Shop")
    void BuyItem(FName ItemRowName, int32 Quantity = 1);

    // === 상점 닫기 ===
    UFUNCTION(BlueprintCallable, Category = "Shop")
    void CloseShop();

    void RefreshSoldItems();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // === Drag & Drop (반환 타입 수정!) ===
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

    // === 탭 전환 ===
    UFUNCTION()
    void OnBuyTabClicked();

    UFUNCTION()
    void OnSellTabClicked();

    void SwitchTab(bool bShowBuyTab);

    // === UI 업데이트 ===
    void DisplayShopItems();
    void DisplayPlayerInventory();
    void UpdateGoldDisplay();

    // === 버튼 이벤트 ===
    UFUNCTION()
    void OnCloseButtonClicked();

    // === 재고 초기화 타이머 ===
    UFUNCTION()
    void OnRestockTimer();

    // === 인벤토리 변경 델리게이트 ===
    UFUNCTION()
    void OnInventoryUpdated();

    UFUNCTION()
    void OnHotbarUpdated();

    // === Widget 컴포넌트 ===

    UPROPERTY(meta = (BindWidget))
    UWrapBox* WB_ShopItems;

    UPROPERTY(meta = (BindWidget))
    UWrapBox* WB_PlayerInventory;

    UPROPERTY(meta = (BindWidget))
    UButton* BTN_BuyTab;

    UPROPERTY(meta = (BindWidget))
    UButton* BTN_SellTab;

    UPROPERTY(meta = (BindWidget))
    UButton* BTN_Close;

    UPROPERTY(meta = (BindWidget))
    UBorder* Border_BuyPanel;

    UPROPERTY(meta = (BindWidget))
    UBorder* Border_SellPanel;

    UPROPERTY(meta = (BindWidget))
    UBorder* Border_LeftPanel;

    UPROPERTY(meta = (BindWidget))
    UBorder* DropZone;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ShopName;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_PlayerGold;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_DropHint;

    // === Widget 클래스 ===

    UPROPERTY(EditDefaultsOnly, Category = "Shop")
    TSubclassOf<UShopItemSlot> ShopItemSlotClass;

    UPROPERTY(EditDefaultsOnly, Category = "Shop")
    TSubclassOf<UInventoryItemSlot> InventoryItemSlotClass;

    UPROPERTY(meta = (BindWidget))
    UScrollBox* SB_SoldItems;

private:
    // === 참조 ===

    UPROPERTY()
    ANPCCharacter* NPCCharacter;

    UPROPERTY()
    UInventoryComponent* PlayerInventory;

    UPROPERTY()
    TArray<FSoldItemRecord> SoldItemHistory;

    // === 상태 ===
    bool bShowBuyTab;

    // === 타이머 ===
    FTimerHandle RestockTimerHandle;

protected:
    // DropZone 색상 (WBP에서 편집 가능)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop|Colors")
    FLinearColor DropZoneDefaultColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop|Colors")
    FLinearColor DropZoneCanSellColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop|Colors")
    FLinearColor DropZoneCannotSellColor = FLinearColor::Red;

    // 탭 텍스트 색상 (WBP에서 편집 가능)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop|Colors")
    FSlateColor TabActiveTextColor = FSlateColor(FLinearColor(1.f, 0.8f, 0.f)); // 노란색 예시

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop|Colors")
    FSlateColor TabInactiveTextColor = FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)); // 회색 예시

    // 탭 텍스트 블록 바인딩 (WBP에서 연결)
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* TXT_BuyTab;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    UTextBlock* TXT_SellTab;
};