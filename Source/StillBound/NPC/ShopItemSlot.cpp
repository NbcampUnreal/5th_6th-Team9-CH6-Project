#include "ShopItemSlot.h"
#include "ShopComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UShopItemSlot::NativeConstruct()
{
    Super::NativeConstruct();

    if (BTN_Buy)
    {
        BTN_Buy->OnClicked.AddDynamic(this, &UShopItemSlot::OnBuyButtonClicked);
    }
}

void UShopItemSlot::SetShopItemData(const FShopItemData& Data, UShopComponent* Shop)
{
    ItemData = Data;
    ShopComponentRef = Shop;

    if (!ShopComponentRef) return;

    const int32 BuyPrice = ShopComponentRef->GetBuyPrice(Data.ItemID);
    const int32 Stock = ShopComponentRef->GetItemStock(Data.ItemID);

    if (TXT_ItemName)
    {
        TXT_ItemName->SetText(FText::FromName(Data.ItemID));
    }

    if (TXT_ItemPrice)
    {
        TXT_ItemPrice->SetText(FText::Format(
            FText::FromString(TEXT("{0}G")), BuyPrice));
    }

    if (TXT_ItemStock)
    {
        FText StockText = Stock < 0
            ? FText::FromString(TEXT("¹«ÇÑ"))
            : FText::AsNumber(Stock);
        TXT_ItemStock->SetText(StockText);
    }
}

void UShopItemSlot::OnBuyButtonClicked()
{
    if (!ShopComponentRef) return;

    int32 GoldSpent = 0;
    if (ShopComponentRef->BuyItemFromShop(ItemData.ItemID, 1, GoldSpent))
    {
        UE_LOG(LogTemp, Log, TEXT("Purchased %s for %d gold"),
            *ItemData.ItemID.ToString(), GoldSpent);
    }
}