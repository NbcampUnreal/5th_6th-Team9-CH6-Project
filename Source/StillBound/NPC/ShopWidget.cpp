// ShopWidget.cpp
#include "ShopWidget.h"
#include "NPC/NPCCharacter.h"
#include "Inventory/InventoryComponent.h"
#include "ShopItemSlot.h"
#include "UI/Inventory/InventoryItemSlot.h"
#include "Character/PlayerCharacter_SB.h"
#include "Data/ItemData.h"
#include "Items/ItemBase.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/WrapBox.h"
#include "Components/Border.h"
#include "Kismet/GameplayStatics.h"

void UShopWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UE_LOG(LogTemp, Error, TEXT("========== NativeConstruct CALLED =========="));

    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeUIOnly InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(true);

        UE_LOG(LogTemp, Error, TEXT("Input mode set to UI Only"));
        UE_LOG(LogTemp, Error, TEXT("Mouse cursor visible: %s"),
            PC->ShouldShowMouseCursor() ? TEXT("true") : TEXT("false"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController is NULL!"));
    }

    // 버튼 바인딩
    if (BTN_Close)
    {
        UE_LOG(LogTemp, Log, TEXT("BTN_Close found, binding..."));
        BTN_Close->OnClicked.AddDynamic(this, &UShopWidget::OnCloseButtonClicked);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BTN_Close is NULL!"));
    }

    if (BTN_BuyTab)
    {
        BTN_BuyTab->OnClicked.AddDynamic(this, &UShopWidget::OnBuyTabClicked);
    }

    if (BTN_SellTab)
    {
        BTN_SellTab->OnClicked.AddDynamic(this, &UShopWidget::OnSellTabClicked);
    }

    SwitchTab(true);

    UE_LOG(LogTemp, Error, TEXT("========== NativeConstruct FINISHED =========="));
}

void UShopWidget::NativeDestruct()
{
    // 델리게이트 해제
    if (PlayerInventory)
    {
        PlayerInventory->OnInventoryUpdated.RemoveAll(this);
    }

    Super::NativeDestruct();
}

void UShopWidget::InitializeShop(ANPCCharacter* NPC)
{
    if (!NPC)
    {
        UE_LOG(LogTemp, Error, TEXT("ShopWidget: Invalid NPC"));
        return;
    }

    NPCCharacter = NPC;

    // 플레이어 인벤토리
    APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

    if (Player)
    {
        PlayerInventory = Player->GetInventory();
    }

    if (!PlayerInventory)
    {
        UE_LOG(LogTemp, Error, TEXT("ShopWidget: Player inventory not found"));
        return;
    }

    // 상점 이름
    if (TXT_ShopName)
    {
        TXT_ShopName->SetText(NPCCharacter->ShopName);
    }

    // 델리게이트 바인딩
    PlayerInventory->OnInventoryUpdated.AddUObject(this, &UShopWidget::RefreshShop);

    // 초기 표시
    RefreshShop();

    UE_LOG(LogTemp, Log, TEXT("ShopWidget: Initialized shop '%s'"), *NPCCharacter->ShopName.ToString());

    // 재고 리셋 타이머 시작
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            RestockTimerHandle,
            this,
            &UShopWidget::OnRestockTimer,
            RestockIntervalSeconds,
            true  // 반복
        );

        UE_LOG(LogTemp, Log, TEXT("Restock timer started (%.0f seconds)"),
            RestockIntervalSeconds);
    }
}

void UShopWidget::OnRestockTimer()
{
    if (!NPCCharacter) return;

    // NPC의 상점 아이템 재초기화
    NPCCharacter->InitializeShopItems();

    // UI 새로고침
    RefreshShop();

    UE_LOG(LogTemp, Log, TEXT("Shop restocked!"));
}


void UShopWidget::CloseShop()
{
    // 입력 모드 복원
    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(false);
    }

    RemoveFromParent();

    UE_LOG(LogTemp, Log, TEXT("ShopWidget: Closed"));
}

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

bool UShopWidget::BuyItem(FName ItemID, int32 Quantity)
{
    if (!NPCCharacter || !PlayerInventory) return false;

    // 1. 가격 계산
    int32 UnitPrice = NPCCharacter->GetItemPrice(ItemID);
    int32 TotalPrice = UnitPrice * Quantity;

    // 2. 골드 체크
    APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

    if (!Player || !Player->HasEnoughGold(TotalPrice))
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough gold! Need: %d, Have: %d"),
            TotalPrice, Player ? Player->GetGold() : 0);

        // TODO: "골드 부족" UI 표시
        return false;
    }
    //  3. 아이템 구매 시도
    if (NPCCharacter->SellItemToPlayer(ItemID, Quantity))
    {
        // 4. 골드 차감
        Player->ModifyGold(-TotalPrice);

        UE_LOG(LogTemp, Log, TEXT("Bought %d x %s for %d gold"),
            Quantity, *ItemID.ToString(), TotalPrice);

        // 5. UI 새로고침
        RefreshShop();
        return true;
    }

    return false;
}

bool UShopWidget::SellItem(UItemBase* Item, int32 Quantity)
{
    if (!NPCCharacter || !Item) return false;

    int32 GoldReceived = 0;

    // NPC에게 아이템 판매
    if (NPCCharacter->BuyItemFromPlayer(Item, Quantity, GoldReceived))
    {
        // 골드 지급
        APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(
            UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

        if (Player)
        {
            Player->ModifyGold(GoldReceived);
        }

        UE_LOG(LogTemp, Log, TEXT("Sold %d x %s for %d gold"),
            Quantity, *Item->TextData.Name.ToString(), GoldReceived);

        // UI 새로고침
        RefreshShop();
        return true;
    }

    return false;
}

void UShopWidget::OnCloseButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("ShopWidget: Close button clicked"));

    // 상점 닫기
    RemoveFromParent();

    // Input Mode 복원
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(false);

        UE_LOG(LogTemp, Log, TEXT("ShopWidget: Input mode restored to Game"));
    }
}

void UShopWidget::OnBuyTabClicked()
{
    SwitchTab(true);
}

void UShopWidget::OnSellTabClicked()
{
    SwitchTab(false);
}

void UShopWidget::DisplayShopItems()
{
    if (!WB_ShopItems || !ShopItemSlotClass || !NPCCharacter) return;

    WB_ShopItems->ClearChildren();

    // Getter 함수로 가져오기!
    const TArray<FName>& ShopItems = NPCCharacter->GetShopItemList();

    UE_LOG(LogTemp, Log, TEXT("Shop items count: %d"), ShopItems.Num());

    // 순회
    for (const FName& ItemID : ShopItems)
    {
        UE_LOG(LogTemp, Log, TEXT("Processing item: %s"), *ItemID.ToString());

        FItemDataRow* ItemData = NPCCharacter->GetItemData(ItemID);
        if (!ItemData)
        {
            UE_LOG(LogTemp, Error, TEXT("  ItemData is NULL for %s"), *ItemID.ToString());
            continue;
        }

        int32 Price = NPCCharacter->GetItemPrice(ItemID);

        UShopItemSlot* ItemSlot = CreateWidget<UShopItemSlot>(this, ShopItemSlotClass);
        if (ItemSlot)
        {
            // ItemID (RowName)을 함께 전달!
            ItemSlot->SetShopItemData(ItemID, ItemData, Price, NPCCharacter);
            WB_ShopItems->AddChildToWrapBox(ItemSlot);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ShopWidget: Displayed %d shop items"), ShopItems.Num());
}

void UShopWidget::DisplayPlayerInventory()
{
    if (!WB_PlayerInventory || !InventoryItemSlotClass || !PlayerInventory) return;

    WB_PlayerInventory->ClearChildren();

    const TArray<TObjectPtr<UItemBase>>& Items = PlayerInventory->GetInventorySlots();

    for (UItemBase* Item : Items)
    {
        // nullptr 체크 (빈 슬롯 제외)
        if (!Item) continue;

        UInventoryItemSlot* ItemSlot = CreateWidget<UInventoryItemSlot>(
            this, InventoryItemSlotClass);

        if (ItemSlot)
        {
            ItemSlot->SetItemReference(Item);
            WB_PlayerInventory->AddChildToWrapBox(ItemSlot);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ShopWidget: Displayed %d player items"),
        Items.Num());
}

void UShopWidget::UpdateGoldDisplay()
{
    if (TXT_PlayerGold)
    {
        // TODO: 실제 골드 시스템 연동
        TXT_PlayerGold->SetText(FText::FromString(TEXT("Gold: 1000")));
    }
}

void UShopWidget::SwitchTab(bool bBuyTab)
{
    bShowBuyTab = bBuyTab;

    if (Border_BuyPanel && Border_SellPanel)
    {
        Border_BuyPanel->SetVisibility(bBuyTab ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        Border_SellPanel->SetVisibility(bBuyTab ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    }

    RefreshShop();
}