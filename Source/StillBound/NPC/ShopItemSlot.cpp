#include "ShopItemSlot.h"
#include "NPC/NPCCharacter.h"
#include "Data/ItemData.h"
#include "Items/ItemBase.h"
#include "UI/Inventory/InventoryTooltip.h"
#include "UI/Inventory/InventoryItemSlot.h" 
#include "ShopWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UShopItemSlot::NativeConstruct()
{
    Super::NativeConstruct();

    // 초기화
    CachedItemData = nullptr;
    CachedPrice = 0;
    ShopItemTooltip = nullptr;

    // 버튼 바인딩
    if (BTN_Buy)
    {
        //BTN_Buy->OnClicked.AddDynamic(this, &UShopItemSlot::OnBuyButtonClicked);
        BTN_Buy->OnClicked.AddUniqueDynamic(this, &UShopItemSlot::OnBuyButtonClicked);
    }
}

void UShopItemSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    if (!TooltipClass || !NPCRef) return;

    // 캐시된 데이터가 없으면 리턴
    if (!CachedItemData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ShopItemSlot] No cached item data for tooltip"));
        return;
    }

    // ============================================
    // Tooltip 생성
    // ============================================
    if (!ShopItemTooltip)
    {
        ShopItemTooltip = CreateWidget<UInventoryTooltip>(this, TooltipClass);
        if (!ShopItemTooltip)
        {
            UE_LOG(LogTemp, Error, TEXT("[ShopItemSlot] Failed to create tooltip!"));
            return;
        }
    }
    // ============================================
    // 임시 InventoryItemSlot 생성 (RefreshFromSlot에 필요)
    // ============================================
    UInventoryItemSlot* TempSlot = NewObject<UInventoryItemSlot>(this);
    if (!TempSlot)
    {
        UE_LOG(LogTemp, Error, TEXT("[ShopItemSlot] Failed to create temp slot!"));
        return;
    }
    // ============================================
    // 임시 ItemBase 생성 (InventoryTooltip은 ItemBase를 받음)
    // ============================================
    UItemBase* TempItem = NewObject<UItemBase>(this);
    if (!TempItem)
    {
        UE_LOG(LogTemp, Error, TEXT("[ShopItemSlot] Failed to create temp item!"));
        return;
    }

    // ItemBase에 데이터 복사
    TempItem->ID = CachedItemData->ID;
    TempItem->ItemType = CachedItemData->ItemType;
    TempItem->ItemQuality = CachedItemData->ItemQuality;
    TempItem->NumericData = CachedItemData->NumericData;
    TempItem->TextData = CachedItemData->TextData;
    TempItem->AssetData = CachedItemData->AssetData;
    TempItem->ItemStatistics = CachedItemData->ItemStatistics;
    TempItem->SetQuantity(1);

    // ============================================
    // Tooltip 업데이트 (가격 정보 추가)
    // ============================================

    // TempSlot에 TempItem 설정
    TempSlot->SetItemReference(TempItem);

    // ============================================
    // Tooltip에 Slot 설정 후 RefreshFromSlot 호출
    // ============================================
    ShopItemTooltip->InventorySlotBeingHovered = TempSlot;
    ShopItemTooltip->RefreshFromSlot();


    // Viewport에 추가
    ShopItemTooltip->AddToViewport(999);

    UE_LOG(LogTemp, Log, TEXT("[ShopItemSlot] Tooltip shown: %s (%dG)"),
        *CachedItemData->TextData.Name.ToString(),
        CachedPrice);
}

void UShopItemSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    // Tooltip 제거
    if (ShopItemTooltip && ShopItemTooltip->IsInViewport())
    {
        ShopItemTooltip->RemoveFromParent();

        UE_LOG(LogTemp, Log, TEXT("[ShopItemSlot] Tooltip hidden"));
    }
}

void UShopItemSlot::SetShopItemData(const FShopItemData& InItemData, UShopWidget* InShopWidget, ANPCCharacter* InNPC)
{
    ItemData = InItemData;
    ShopWidget = InShopWidget;
    NPCRef = InNPC;

    if (!ShopWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("[ShopItemSlot] ShopWidget is NULL!"));
        return;
    }
    if (!NPCRef)
    {
        UE_LOG(LogTemp, Error, TEXT("[ShopItemSlot] NPCRef is NULL!"));
        return;
    }

    // NPC에서 아이템 데이터 가져오기
    const FItemDataRow* FullItemData = NPCRef->GetItemData(ItemData.ItemRowName);
    if (!FullItemData)
    {
        UE_LOG(LogTemp, Error, TEXT("[ShopItemSlot] Item data not found: %s"),
            *ItemData.ItemRowName.ToString());

        // 기본값 표시
        if (TXT_ItemName)
        {
            TXT_ItemName->SetText(FText::FromString(TEXT("Unknown Item")));
        }
        if (TXT_ItemPrice)
        {
            TXT_ItemPrice->SetText(FText::FromString(TEXT("Price: 0G")));
        }
        return;
    }

    // ============================================
    // 데이터 캐싱 (Tooltip에서 사용)
    // ============================================
    CachedItemData = FullItemData;
    CachedPrice = FullItemData->ItemStatistics.SellValue;

    // ============================================
    // UI 업데이트
    // ============================================

    // 아이템 이름
    if (TXT_ItemName)
    {
        TXT_ItemName->SetText(FullItemData->TextData.Name);
    }

    // 가격 표시
    if (TXT_ItemPrice)
    {
        FText PriceText = FText::Format(
            FText::FromString(TEXT("{0}G")),
            FText::AsNumber(CachedPrice)
        );
        TXT_ItemPrice->SetText(PriceText);
    }
    //재고
    if (TXT_ItemStock)
    {
        if (ItemData.CurrentStock < 0)
        {
            TXT_ItemStock->SetText(FText::FromString( TEXT("Infi_Stock")));
        }
        else
        {
            FText StockText = FText::Format(
                FText::FromString(TEXT("{0}G")),
                FText::AsNumber(ItemData.CurrentStock)
            );
            TXT_ItemStock->SetText(StockText);
        }
    }

    // 아이콘
    if (IMG_ItemIcon && FullItemData->AssetData.Icon)
    {
        IMG_ItemIcon->SetBrushFromTexture(FullItemData->AssetData.Icon);
    }

    // 품절 표시
    if (ItemData.CurrentStock <= 0)
    {
        // 버튼 비활성화
        if (BTN_Buy)
        {
            BTN_Buy->SetIsEnabled(false);
        }

        // 아이템 이름에 "(품절)" 추가
        if (TXT_ItemName)
        {
            FText OutOfStockText = FText::Format(
                FText::FromString(TEXT("{0} (Out Of Stock)")),
                FullItemData->TextData.Name
            );
            TXT_ItemName->SetText(OutOfStockText);
        }

        // 가격 텍스트 회색으로
        if (TXT_ItemPrice)
        {
            TXT_ItemPrice->SetColorAndOpacity(FLinearColor(0.4f, 0.4f, 0.4f, 1.0f));
        }
        if (TXT_ItemStock)
        {
            TXT_ItemStock->SetText(FText::FromString(TEXT("Out Of Stock")));
            TXT_ItemStock->SetColorAndOpacity(
                FSlateColor(FLinearColor(0.75f, 0.38f, 0.38f, 1.0f)));
        }
    }
}

void UShopItemSlot::OnBuyButtonClicked()
{
    // 1. 기본 유효성 검사
    if (!ShopWidget || ItemData.ItemRowName == NAME_None)
    {
        return;
    }

    // 2. 품절 체크 (정확히 0일 때만 막아야 무한 재고를 살 수 있음)
    if (ItemData.CurrentStock == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ShopItemSlot] Item out of stock!"));
        return;
    }

    // 3. 플레이어 컨트롤러 및 키 입력 감지
    const UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    bool bShiftPressed = PC->IsInputKeyDown(EKeys::LeftShift) ||
        PC->IsInputKeyDown(EKeys::RightShift);

    // 4. 구매 수량 결정 로직
    int32 PurchaseQuantity = 1; // 기본은 1개

    if (bShiftPressed)
    {
        // 묶음 구매 시 최대 수량 설정
        const int32 MaxBundleSize = 5;

        if (ItemData.CurrentStock < 0)
        {
            // 무한 재고인 경우: 그냥 5개 꽉 채워서 구매
            PurchaseQuantity = MaxBundleSize;
        }
        else
        {
            // 유한 재고인 경우: 남은 재고와 5개 중 작은 값 선택
            PurchaseQuantity = FMath::Min(ItemData.CurrentStock, MaxBundleSize);
        }

        UE_LOG(LogTemp, Log, TEXT("[ShopItemSlot] Shift+Click: Buying %d items"), PurchaseQuantity);
    }

    // 5. 상점 위젯에 최종 구매 요청
    ShopWidget->BuyItem(ItemData.ItemRowName, PurchaseQuantity);
} 