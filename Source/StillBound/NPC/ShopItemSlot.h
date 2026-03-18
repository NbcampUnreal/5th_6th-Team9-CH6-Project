// ShopItemSlot.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NPC/NPCCharacter.h"
#include "Components/Button.h"
#include "ShopItemSlot.generated.h"

class UImage;
class UTextBlock;
class UButton;
class UBorder;
class UInventoryTooltip;
class UShopWidget;
class ANPCCharacter;
struct FItemDataRow;

UCLASS()
class STILLBOUND_API UShopItemSlot : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
    
public:
    UFUNCTION(BlueprintCallable, Category = "Shop")
    void SetShopItemData(const FShopItemData& InItemData, UShopWidget* InShopWidget, ANPCCharacter* InNPC);

    void SetBuyButtonVisible(bool bVisible)
    {
        if (BTN_Buy)
            BTN_Buy->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

protected:
    UPROPERTY(meta = (BindWidget))
    UImage* IMG_ItemIcon;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ItemName;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ItemPrice;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ItemStock;

    UPROPERTY(meta = (BindWidget))
    UButton* BTN_Buy;

    // 툴팁 위젯
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf< UInventoryTooltip> TooltipClass;

private:
    
    UPROPERTY()
    ANPCCharacter* NPCRef = nullptr;

    UFUNCTION()
    void OnBuyButtonClicked();

    // === 데이터 ===
    FShopItemData ItemData;

    UPROPERTY()
    UShopWidget* ShopWidget;

    UPROPERTY()
    UInventoryTooltip* ShopItemTooltip;

    // 캐시된 아이템 데이터
    const FItemDataRow* CachedItemData;
    int32 CachedPrice;

};