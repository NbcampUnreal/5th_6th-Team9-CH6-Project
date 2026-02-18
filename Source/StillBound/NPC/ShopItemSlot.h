// ShopItemSlot.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopComponent.h"
#include "ShopItemSlot.generated.h"

class UImage;
class UTextBlock;
class UButton;

UCLASS()
class STILLBOUND_API UShopItemSlot : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

public:
    UFUNCTION(BlueprintCallable, Category = "Shop")
    void SetShopItemData(const FShopItemData& Data, UShopComponent* Shop);

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

private:
    UPROPERTY()
    UShopComponent* ShopComponentRef = nullptr;

    FShopItemData ItemData;

    UFUNCTION()
    void OnBuyButtonClicked();
};