// ShopWidget.cpp
#include "ShopWidget.h"
#include "NPC/NPCCharacter.h"
#include "Inventory/InventoryComponent.h"
#include "ShopItemSlot.h"
#include "UI/Inventory/InventoryItemSlot.h"
#include "Character/PlayerCharacter_SB.h"
#include "Data/ItemData.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/WrapBox.h"
#include "Components/Border.h"
#include "Kismet/GameplayStatics.h"

void UShopWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 버튼 바인딩
    if (BTN_Close)
    {
        BTN_Close->OnClicked.AddDynamic(this, &UShopWidget::OnCloseButtonClicked);
    }

    if (BTN_BuyTab)
    {
        BTN_BuyTab->OnClicked.AddDynamic(this, &UShopWidget::OnBuyTabClicked);
    }

    if (BTN_SellTab)
    {
        BTN_SellTab->OnClicked.AddDynamic(this, &UShopWidget::OnSellTabClicked);
    }

    // 입력 모드 UI로 전환
    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(this->TakeWidget());
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(true);
    }

    // 초기 탭 설정
    SwitchTab(true);
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

void UShopWidget::OnCloseButtonClicked()
{
    CloseShop();
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

    for (const FName& ItemID : NPCCharacter->SellableItemIDs)
    {
        FItemDataRow* ItemData = NPCCharacter->GetItemData(ItemID);
        if (!ItemData)
        {
            UE_LOG(LogTemp, Warning, TEXT("Item %s not found in DataTable"), *ItemID.ToString());
            continue;
        }
        // 가격 계산
        int32 Price = NPCCharacter->GetItemPrice(ItemID);

        // 슬롯 생성
        UShopItemSlot* ItemSlot = CreateWidget<UShopItemSlot>(this, ShopItemSlotClass);
        if (ItemSlot)
        {
            ItemSlot->SetShopItemData(ItemData, Price, NPCCharacter);
            WB_ShopItems->AddChildToWrapBox(ItemSlot);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ShopWidget: Displayed %d shop items"),
        NPCCharacter->SellableItemIDs.Num());
}

void UShopWidget::DisplayPlayerInventory()
{
    if (!WB_PlayerInventory || !InventoryItemSlotClass || !PlayerInventory) return;

    WB_PlayerInventory->ClearChildren();

    for (UItemBase* Item : PlayerInventory->GetInventoryContents())
    {
        UInventoryItemSlot* ItemSlot = CreateWidget<UInventoryItemSlot>(this, InventoryItemSlotClass);
        if (ItemSlot)
        {
            ItemSlot->SetItemReference(Item);
            // TODO: 판매 기능 활성화 (ItemSlot에 판매 버튼 추가)
            WB_PlayerInventory->AddChildToWrapBox(ItemSlot);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ShopWidget: Displayed %d player items"),
        PlayerInventory->GetInventoryContents().Num());
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