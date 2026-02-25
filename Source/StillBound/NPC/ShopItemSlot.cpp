#include "ShopItemSlot.h"
#include "NPC/NPCCharacter.h"
#include "Data/ItemData.h"
#include "Shop/ShopTooltip.h"
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

void UShopItemSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    if (!TooltipClass || !NPCRef) return;

    // 툴팁 생성
    if (!ShopTooltipWidget)
    {
        ShopTooltipWidget = CreateWidget<UShopTooltip>(this, TooltipClass);
    }

    if (ShopTooltipWidget)
    {
        // 아이템 정보 + 가격 표시
        FItemDataRow* ItemData = NPCRef->GetItemData(ItemID);
        if (ItemData)
        {
            ShopTooltipWidget->SetItemInfo(ItemData, Price);
            ShopTooltipWidget->AddToViewport(999);
        }
    }
}

void UShopItemSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    // 툴팁 제거
    if (ShopTooltipWidget && ShopTooltipWidget->IsInViewport())
    {
        ShopTooltipWidget->RemoveFromParent();
    }
}

void UShopItemSlot::SetShopItemData(FItemDataRow* ItemData, int32 InPrice, ANPCCharacter* NPC)
{
    if (!ItemData || !NPC) return;

    ItemID = ItemData->ID;
    Price = InPrice;
    NPCRef = NPC;

    if (TXT_ItemName)
    {
        TXT_ItemName->SetText(FText::FromName(ItemID));
    }

    if (TXT_ItemPrice)
    {
        TXT_ItemPrice->SetText(FText::Format(
            FText::FromString(TEXT("{0}G")), Price));
    }

    if (TXT_ItemStock)
    {
        FText StockText = Stock < 0
            ? FText::FromString(TEXT("무한"))
            : FText::AsNumber(Stock);
        TXT_ItemStock->SetText(StockText);
    }
}

void UShopItemSlot::OnBuyButtonClicked()
{
    if (!NPCRef) return;

    bool bSuccess = NPCRef->SellItemToPlayer(ItemID, 1);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Purchased %s"), *ItemID.ToString());

        if (Stock > 0)
        {
            Stock--;
            if (TXT_ItemStock)
            {
                TXT_ItemStock->SetText(FText::AsNumber(Stock));

            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ShopItemSlot: Purchase failed"));
    }
} 