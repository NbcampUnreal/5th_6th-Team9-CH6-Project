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

void UShopItemSlot::SetShopItemData(FName InItemID, FItemDataRow* ItemData, int32 InPrice, ANPCCharacter* NPC)
{
    if (!ItemData || !NPC)
    {
        UE_LOG(LogTemp, Error, TEXT("SetItemData: ItemData or NPC is NULL!"));
        return;
    }

    //ItemID = ItemData->ID;
    ItemID = InItemID;
    Price = InPrice;
    NPCRef = NPC;

    // UI 업데이트
    if (TXT_ItemName)
    {
        FText ItemName = ItemData->TextData.Name;

        // 이름이 비어있으면 경고
        if (ItemName.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("Item name is empty!"));
            ItemName = FText::FromString(TEXT("Unknown Item"));
        }

        TXT_ItemName->SetText(ItemName);
        UE_LOG(LogTemp, Log, TEXT("  Set ItemName: %s"), *ItemName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("  TXT_ItemName is NULL!"));
    }

    if (TXT_ItemPrice)
    {
        TXT_ItemPrice->SetText(FText::Format(
            FText::FromString(TEXT("{0}G")), Price));
        UE_LOG(LogTemp, Log, TEXT("  Set Price: %d"), Price);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("  TXT_ItemPrice is NULL!"));
    }

    if (IMG_ItemIcon)
    {
        if (ItemData->AssetData.Icon)
        {
            IMG_ItemIcon->SetBrushFromTexture(ItemData->AssetData.Icon);
        }
        else
        {
            IMG_ItemIcon->SetColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
        }
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