// ShopItemSlot.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopItemSlot.generated.h"

class UImage;
class UTextBlock;
class UButton;
class ANPCCharacter;
struct FItemDataRow;

UCLASS()
class STILLBOUND_API UShopItemSlot : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

public:
    void SetShopItemData(FItemDataRow* ItemData, int32 InPrice, ANPCCharacter* NPC);

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
    ANPCCharacter* NPCRef = nullptr;

    FName ItemID;
    int32 Price = 0;
    int32 Stock = -1;

    UFUNCTION()
    void OnBuyButtonClicked();
};