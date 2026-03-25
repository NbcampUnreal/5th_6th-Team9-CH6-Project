// ShopWidget.cpp

#include "NPC/ShopWidget.h"
#include "NPC/NPCCharacter.h"
#include "NPC/ShopItemSlot.h"
#include "Character/PlayerCharacter_SB.h"
#include "Inventory/InventoryComponent.h"
#include "UI/Inventory/InventoryItemSlot.h"
#include "Items/ItemBase.h"
#include "UI/Inventory/ItemDragDropOperation.h"
#include "Components/WrapBox.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Blueprint/WidgetBlueprintLibrary.h"  

void UShopWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UE_LOG(LogTemp, Warning, TEXT("[ShopWidget] NativeConstruct CALLED"));

    // 입력 모드 설정
    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(TakeWidget());
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }

    // 버튼 바인딩
    if (BTN_BuyTab)
    {
        BTN_BuyTab->OnClicked.AddUniqueDynamic(this, &UShopWidget::OnBuyTabClicked);
    }

    if (BTN_SellTab)
    {
        BTN_SellTab->OnClicked.AddUniqueDynamic(this, &UShopWidget::OnSellTabClicked);
    }

    if (BTN_Close)
    {
        BTN_Close->OnClicked.AddUniqueDynamic(this, &UShopWidget::OnCloseButtonClicked);
    }

    // 기본 탭: Buy
    SwitchTab(true);
}

void UShopWidget::NativeDestruct()
{
    // 인벤토리 델리게이트 해제
    if (PlayerInventory)
    {
        PlayerInventory->OnInventoryUpdated.RemoveAll(this);
        PlayerInventory->OnHotbarUpdated.RemoveAll(this);
    }

    // 타이머 정리
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(RestockTimerHandle);
    }

    Super::NativeDestruct();
}

// ============================================
// InitializeShop
// ============================================
void UShopWidget::InitializeShop(ANPCCharacter* InNPCCharacter)
{
    if (!InNPCCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("[ShopWidget] NPCCharacter is NULL!"));
        return;
    }

    NPCCharacter = InNPCCharacter;

    // Player 가져오기
    APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(GetOwningPlayerPawn());
    if (!Player)
    {
        UE_LOG(LogTemp, Error, TEXT("[ShopWidget] Failed to get Player!"));
        return;
    }
    if (PlayerInventory)
    {
        PlayerInventory->OnInventoryUpdated.RemoveAll(this);
        PlayerInventory->OnHotbarUpdated.RemoveAll(this);
    }

    PlayerInventory = Player->GetInventory();
    if (!PlayerInventory)
    {
        UE_LOG(LogTemp, Error, TEXT("[ShopWidget] PlayerInventory is NULL!"));
        return;
    }

    // 인벤토리 업데이트 델리게이트 바인딩
    PlayerInventory->OnInventoryUpdated.AddUniqueDynamic(this, &UShopWidget::OnInventoryUpdated);
    PlayerInventory->OnHotbarUpdated.AddUObject(this, &UShopWidget::OnInventoryUpdated);

    UE_LOG(LogTemp, Warning, TEXT("[ShopWidget] Inventory delegates bound"));

    // 상점 이름 설정
    if (TXT_ShopName)
    {
        TXT_ShopName->SetText(NPCCharacter->ShopName);
    }

    // 상점 새로고침
    RefreshShop();

    // 재고 초기화 타이머 (10분 = 600초)
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            RestockTimerHandle,
            this,
            &UShopWidget::OnRestockTimer,
            600.0f,  // 10분 고정
            true     // 반복
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("[ShopWidget] InitializeShop complete: '%s'"),
        *NPCCharacter->ShopName.ToString());
}

// ============================================
// RefreshShop
// ============================================
void UShopWidget::RefreshShop()
{
    if (bShowBuyTab)
    {
        DisplayShopItems();
    }
    else
    {
        DisplayPlayerInventory();
    }

    UpdateGoldDisplay();
}

// ============================================
// DisplayShopItems
// ============================================
void UShopWidget::DisplayShopItems()
{
    if (!WB_ShopItems || !NPCCharacter || !ShopItemSlotClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[ShopWidget] DisplayShopItems: Missing components"));
        return;
    }

    WB_ShopItems->ClearChildren();

    const TArray<FShopItemData>& ShopItems = NPCCharacter->GetShopItemList();

    UE_LOG(LogTemp, Warning, TEXT("[ShopWidget] DisplayShopItems: %d items"), ShopItems.Num());

    for (const FShopItemData& ShopItem : ShopItems)
    {
        UShopItemSlot* ItemSlot = CreateWidget<UShopItemSlot>(this, ShopItemSlotClass);
        if (ItemSlot)
        {
            ItemSlot->SetShopItemData(ShopItem, this, NPCCharacter);

            // 품절이면 버튼 비활성화
            if (ShopItem.CurrentStock <= 0)
            {
                ItemSlot->SetIsEnabled(false);
                UE_LOG(LogTemp, Warning, TEXT("  Item out of stock: %s"),
                    *ShopItem.ItemRowName.ToString());
            }

            WB_ShopItems->AddChildToWrapBox(ItemSlot);
        }
    }
}

// ============================================
// DisplayPlayerInventory
// ============================================
void UShopWidget::DisplayPlayerInventory()
{
    if (!WB_PlayerInventory || !PlayerInventory || !InventoryItemSlotClass) return;

    WB_PlayerInventory->ClearChildren();

    const TArray<UItemBase*>& HotbarSlots = PlayerInventory->GetHotbarSlots();
    for (int32 i = 0; i < HotbarSlots.Num(); ++i)
    {
        if (!HotbarSlots[i]) continue; // 빈 슬롯은 스킵 (판매 탭이니 아이템 있는 것만)

        UInventoryItemSlot* ItemSlot = CreateWidget<UInventoryItemSlot>(this, InventoryItemSlotClass);
        if (ItemSlot)
        {
            ItemSlot->InitSlot(ESlotContainer::Hotbar, i, PlayerInventory);
            ItemSlot->SetItemReference(HotbarSlots[i]);
            WB_PlayerInventory->AddChildToWrapBox(ItemSlot);
        }
    }

    const TArray<UItemBase*>& InventorySlots = PlayerInventory->GetInventorySlots();
    for (int32 i = 0; i < InventorySlots.Num(); ++i)
    {
        UInventoryItemSlot* ItemSlot = CreateWidget<UInventoryItemSlot>(this, InventoryItemSlotClass);
        if (ItemSlot)
        {
            ItemSlot->InitSlot(ESlotContainer::Inventory, i, PlayerInventory);

            if (InventorySlots[i])
            {
                ItemSlot->SetItemReference(InventorySlots[i]);
            }

            WB_PlayerInventory->AddChildToWrapBox(ItemSlot);
        }
    }
}

// ============================================
// UpdateGoldDisplay
// ============================================
void UShopWidget::UpdateGoldDisplay()
{
    if (!TXT_PlayerGold)
    {
        return;
    }

    APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(GetOwningPlayerPawn());
    if (Player)
    {
        FText GoldText = FText::Format(
            FText::FromString(TEXT("Gold: {0}")),
            FText::AsNumber(Player->GetGold())
        );
        TXT_PlayerGold->SetText(GoldText);
    }
}

// ============================================
// BuyItem
// ============================================
void UShopWidget::BuyItem(FName ItemRowName, int32 Quantity)
{
    if (!NPCCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("[ShopWidget] BuyItem: NPCCharacter is NULL"));
        return;
    }

    // 아이템 데이터 조회
    const FItemDataRow* ItemData = NPCCharacter->GetItemData(ItemRowName);
    if (!ItemData)
    {
        UE_LOG(LogTemp, Error, TEXT("[ShopWidget] Item not found: %s"), *ItemRowName.ToString());
        return;
    }

    const int32 UnitPrice = ItemData->ItemStatistics.SellValue;
    const int32 TotalPrice = UnitPrice * Quantity;

    APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(GetOwningPlayerPawn());
    if (!Player)
    {
        UE_LOG(LogTemp, Error, TEXT("[ShopWidget] Player not found"));
        return;
    }

    // 골드 체크
    if (Player->GetGold() < TotalPrice)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ShopWidget] Not enough gold! Need %dG, have %dG"),
            TotalPrice, Player->GetGold());
        return;
    }

    // NPC에서 아이템 판매
    if (NPCCharacter->SellItemToPlayer(Player, ItemRowName, Quantity))
    {
        // 골드 차감
        Player->ModifyGold(-TotalPrice);

        // UI 업데이트
        UpdateGoldDisplay();
        RefreshShop();

        UE_LOG(LogTemp, Log, TEXT("[ShopWidget] Purchased %dx %s for %dG"),
            Quantity, *ItemRowName.ToString(), TotalPrice);
    }
}

// ============================================
// NativeOnDrop
// ============================================
bool UShopWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    UItemDragDropOperation* DragOp = Cast<UItemDragDropOperation>(InOperation);
    if (!DragOp || !DragOp->SourceItem)
    {
        return false;
    }

    UItemBase* DraggedItem = DragOp->SourceItem;

    const int32 BaseSellValue = DraggedItem->ItemStatistics.SellValue;
    const int32 SellPrice = FMath::FloorToInt(BaseSellValue * 0.8f);

    // 판매 불가 체크
    if (SellPrice <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ShopWidget] This item cannot be sold: %s"),
            *DraggedItem->TextData.Name.ToString());

        if (DropZone)
        {
            DropZone->SetBrushColor(FLinearColor::White);
        }
        if (TXT_DropHint)
        {
            TXT_DropHint->SetText(FText::FromString(TEXT("Drag item here to sell")));
        }
        return true;
    }

    // Shift 키로 전체 판매 여부 확인
    int32 SellQuantity = InDragDropEvent.IsShiftDown() ? DraggedItem->Quantity : 1;

    FName ItemRowName = DraggedItem->ID;
    FText ItemName = DraggedItem->TextData.Name;

    APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(GetOwningPlayerPawn());
    if (!Player) return false;

    if (NPCCharacter) {
        // NPC에게 아이템 판매
        if (NPCCharacter && NPCCharacter->BuyItemFromPlayer(Player, DraggedItem, SellQuantity))
        {
            const int32 TotalGold = SellPrice * SellQuantity;
            // 플레이어에게 골드 지급
            Player->ModifyGold(TotalGold);

            // UI 업데이트
            UpdateGoldDisplay();

            // 판매 기록 추가
            FSoldItemRecord Record;
            Record.ItemRowName = ItemRowName;
            Record.Quantity = SellQuantity;
            Record.GoldEarned = TotalGold;
            SoldItemHistory.Add(Record);

            // 판매 목록 갱신
            RefreshSoldItems();

            DisplayPlayerInventory();

            UE_LOG(LogTemp, Log, TEXT("[ShopWidget] Sold %dx %s for %dG"),
                SellQuantity,
                *DraggedItem->TextData.Name.ToString(),
                TotalGold);
        }
    }

    // DropZone 색상 초기화
    if (DropZone)
    {
        DropZone->SetBrushColor(FLinearColor::White);
    }

    if (TXT_DropHint)
    {
        TXT_DropHint->SetText(FText::FromString(TEXT("Drag item here to sell")));
    }

    return true;
}

void UShopWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    UItemDragDropOperation* DragOp = Cast<UItemDragDropOperation>(InOperation);
    if (!DragOp || !DragOp->SourceItem)
    {
        return;
    }

    UItemBase* Item = DragOp->SourceItem;

    const int32 SellPrice = FMath::FloorToInt(Item->ItemStatistics.SellValue * 0.8f);

    // DropZone 하이라이트
    if (DropZone)
    {
        if (SellPrice > 0)
        {
            DropZone->SetBrushColor(FLinearColor::Green);
        }
        else
        {
            DropZone->SetBrushColor(FLinearColor::Red);
        }
    }

    // 가격 표시
    if (TXT_DropHint)
    {
        if (SellPrice > 0)
        {
            FText HintText = FText::Format(
                FText::FromString(TEXT("Sell Price: {0}G")),
                FText::AsNumber(SellPrice)
            );
            TXT_DropHint->SetText(HintText);
        }
        else
        {
            TXT_DropHint->SetText(FText::FromString(TEXT("Cannot be sold")));
        }
    }
}

void UShopWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    if (DropZone)
    {
        DropZone->SetBrushColor(FLinearColor::White);
    }

    if (TXT_DropHint)
    {
        TXT_DropHint->SetText(FText::FromString(TEXT("Drag item here to sell")));
    }
}

// ============================================
// 탭 전환
// ============================================
void UShopWidget::OnBuyTabClicked()
{
    SwitchTab(true);
}

void UShopWidget::OnSellTabClicked()
{
    SwitchTab(false);
}

void UShopWidget::SwitchTab(bool bShowBuy)
{
    bShowBuyTab = bShowBuy;

    RefreshShop();

    if (Border_BuyPanel)
    {
        Border_BuyPanel->SetVisibility(bShowBuy ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    if (Border_SellPanel)
    {
        Border_SellPanel->SetVisibility(bShowBuy ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }

    if (Border_LeftPanel)
    {
        Border_LeftPanel->SetVisibility(bShowBuy ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }
}

// ============================================
// 상점 닫기
// ============================================
void UShopWidget::OnCloseButtonClicked()
{
    CloseShop();
}

void UShopWidget::CloseShop()
{
    SoldItemHistory.Empty();

    // 입력 모드 복구
    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
    }

    // 타이머 정리
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(RestockTimerHandle);
    }

    // Widget 제거
    RemoveFromParent();

    UE_LOG(LogTemp, Log, TEXT("[ShopWidget] Shop closed"));
}

void UShopWidget::RefreshSoldItems()
{
    if (!SB_SoldItems || !NPCCharacter) return;
    SB_SoldItems->ClearChildren();

    for (const FSoldItemRecord& Record : SoldItemHistory)
    {
        UE_LOG(LogTemp, Warning, TEXT("[RefreshSoldItems] Adding: %s"), *Record.ItemRowName.ToString());

        UShopItemSlot* ItemSlot = CreateWidget<UShopItemSlot>(this, ShopItemSlotClass);
        if (!ItemSlot)
        {
            UE_LOG(LogTemp, Error, TEXT("[RefreshSoldItems] ItemSlot NULL!"));
            continue;
        }

        const FItemDataRow* ItemData = NPCCharacter->GetItemData(Record.ItemRowName);
        if (!ItemData)
        {
            UE_LOG(LogTemp, Error, TEXT("[RefreshSoldItems] ItemData NULL for: %s"), *Record.ItemRowName.ToString());
            continue;
        }

        FShopItemData TempData;
        TempData.ItemRowName = Record.ItemRowName;
        TempData.CurrentStock = Record.Quantity;
        TempData.MaxStock = Record.Quantity;

        ItemSlot->SetShopItemData(TempData, this, NPCCharacter);
        //ItemSlot->SetBuyButtonVisible(false);

        UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(SB_SoldItems->AddChild(ItemSlot));

        UE_LOG(LogTemp, Warning, TEXT("[RefreshSoldItems] Children count: %d, ScrollSlot valid: %s"),
            SB_SoldItems->GetChildrenCount(),
            ScrollSlot ? TEXT("YES") : TEXT("NO"));
    
    }
}

// ============================================
// 재고 초기화 타이머
// ============================================
void UShopWidget::OnRestockTimer()
{
    if (!NPCCharacter)
    {
        return;
    }

    // 상점 아이템 재초기화
    NPCCharacter->InitializeShopItems();

    // UI 새로고침
    RefreshShop();

    UE_LOG(LogTemp, Log, TEXT("[ShopWidget] Shop restocked!"));
}

// ============================================
// 인벤토리 업데이트 델리게이트
// ============================================
void UShopWidget::OnInventoryUpdated()
{
    // Sell 탭일 때만 새로고침
    if (!bShowBuyTab)
    {
        DisplayPlayerInventory();
    }
}

void UShopWidget::OnHotbarUpdated()
{
    if (!bShowBuyTab)
    {
        DisplayPlayerInventory();
    }
}
