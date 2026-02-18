// ShopComponent.cpp
#include "ShopComponent.h"
#include "Items/ItemBase.h"
#include "Inventory/InventoryComponent.h"
#include "Character/PlayerCharacter_SB.h"
#include "Kismet/GameplayStatics.h"

UShopComponent::UShopComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UShopComponent::BeginPlay()
{
    Super::BeginPlay();

    // NPC 인벤토리 생성
    if (bUseNPCInventory)
    {
        NPCInventory = NewObject<UInventoryComponent>(GetOwner(), UInventoryComponent::StaticClass());
        if (NPCInventory)
        {
            NPCInventory->SetSlotsCapacity(NPCInventorySlots);
            NPCInventory->SetWeightCapacity(NPCInventoryWeightCapacity);
            NPCInventory->RegisterComponent();

            UE_LOG(LogTemp, Log, TEXT("ShopComponent: NPC inventory created (Slots: %d, Weight: %.1f)"),
                NPCInventorySlots, NPCInventoryWeightCapacity);
        }
    }

    // 초기 재고 설정
    for (const FShopItemData& Item : ShopInventory)
    {
        CurrentStock.Add(Item.ItemID, Item.InitialStock);

        // 재생성 타이머 활성화
        if (Item.RestockTime > 0.0f)
        {
            RestockTimers.Add(Item.ItemID, 0.0f);
            SetComponentTickEnabled(true);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ShopComponent: Shop '%s' initialized with %d items"),
        *ShopName.ToString(), ShopInventory.Num());
}

void UShopComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 재고 재생성 타이머 업데이트
    for (auto& Timer : RestockTimers)
    {
        Timer.Value += DeltaTime;

        const FShopItemData* ItemData = ShopInventory.FindByPredicate([&](const FShopItemData& Data)
            {
                return Data.ItemID == Timer.Key;
            });

        if (ItemData && ItemData->RestockTime > 0.0f && Timer.Value >= ItemData->RestockTime)
        {
            RestockItem(Timer.Key);
            Timer.Value = 0.0f;
        }
    }
}

bool UShopComponent::SellItemToShop(UItemBase* Item, int32 Quantity, int32& OutGoldReceived)
{
    if (!Item || Quantity <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("ShopComponent: Invalid item or quantity"));
        return false;
    }

    UInventoryComponent* PlayerInv = GetPlayerInventory();
    if (!PlayerInv)
    {
        return false;
    }

    // 판매 가능 여부 확인
    FText FailReason;
    if (!CanSellItem(Item, Quantity, FailReason))
    {
        UE_LOG(LogTemp, Warning, TEXT("ShopComponent: Cannot sell - %s"), *FailReason.ToString());
        return false;
    }

    // 실제 판매 가능 수량 확인
    UItemBase* PlayerItem = PlayerInv->FindMatchingItem(Item);
    if (!PlayerItem)
    {
        // ID로 찾기
        PlayerItem = PlayerInv->FindNextItemByID(Item);
    }

    if (!PlayerItem || PlayerItem->Quantity < Quantity)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShopComponent: Not enough items in inventory"));
        return false;
    }

    // 가격 계산
    const int32 UnitPrice = GetSellPrice(Item);
    const int32 TotalPrice = UnitPrice * Quantity;

    // InventoryComponent의 RemoveAmountOfItem 사용
    const int32 RemovedAmount = PlayerInv->RemoveAmountOfItem(PlayerItem, Quantity);

    if (RemovedAmount > 0)
    {
        // 골드 지급
        if (ModifyPlayerGold(TotalPrice))
        {
            OutGoldReceived = TotalPrice;

            // NPC 인벤토리에 추가 (사용하는 경우)
            if (bUseNPCInventory && NPCInventory)
            {
                UItemBase* ShopItem = CreateShopItem(Item->ID, RemovedAmount);
                if (ShopItem)
                {
                    FItemAddResult AddResult = NPCInventory->HandleAddItem(ShopItem);

                    if (AddResult.OperationResult == EItemAddResult::IAR_NoItemAdded)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("ShopComponent: NPC inventory full, item discarded"));
                    }
                }
            }
            else
            {
                // 무한 재고 상점은 그냥 재고 증가
                UpdateStock(Item->ID, RemovedAmount);
            }

            OnShopInventoryUpdated.Broadcast();

            UE_LOG(LogTemp, Log, TEXT("ShopComponent: Player sold %d x %s for %d gold"),
                RemovedAmount, *Item->TextData.Name.ToString(), TotalPrice);
            return true;
        }
    }

    return false;
}

bool UShopComponent::BuyItemFromShop(FName ItemID, int32 Quantity, int32& OutGoldSpent)
{
    if (Quantity <= 0)
    {
        return false;
    }

    // 구매 가능 여부 확인
    FText FailReason;
    if (!CanBuyItem(ItemID, Quantity, FailReason))
    {
        UE_LOG(LogTemp, Warning, TEXT("ShopComponent: Cannot buy - %s"), *FailReason.ToString());
        return false;
    }

    // 가격 계산
    const int32 UnitPrice = GetBuyPrice(ItemID);
    const int32 TotalPrice = UnitPrice * Quantity;

    // 골드 확인 및 차감
    if (!ModifyPlayerGold(-TotalPrice))
    {
        UE_LOG(LogTemp, Warning, TEXT("ShopComponent: Not enough gold (need %d)"), TotalPrice);
        return false;
    }

    UInventoryComponent* PlayerInv = GetPlayerInventory();
    if (!PlayerInv)
    {
        ModifyPlayerGold(TotalPrice); // 실패 시 골드 반환
        return false;
    }

    // 아이템 생성
    UItemBase* NewItem = CreateShopItem(ItemID, Quantity);
    if (!NewItem)
    {
        ModifyPlayerGold(TotalPrice);
        UE_LOG(LogTemp, Error, TEXT("ShopComponent: Failed to create item %s"), *ItemID.ToString());
        return false;
    }

    // InventoryComponent의 HandleAddItem 사용
    FItemAddResult AddResult = PlayerInv->HandleAddItem(NewItem);

    if (AddResult.OperationResult != EItemAddResult::IAR_NoItemAdded)
    {
        OutGoldSpent = TotalPrice;

        // 재고 감소
        UpdateStock(ItemID, -AddResult.ActualAmountAdded);

        OnShopInventoryUpdated.Broadcast();

        UE_LOG(LogTemp, Log, TEXT("ShopComponent: Player bought %d x %s for %d gold"),
            AddResult.ActualAmountAdded, *ItemID.ToString(), TotalPrice);

        // 일부만 추가된 경우 나머지 골드 반환
        if (AddResult.OperationResult == EItemAddResult::IAR_PartialAmountItemAdded)
        {
            const int32 RefundAmount = (Quantity - AddResult.ActualAmountAdded) * UnitPrice;
            ModifyPlayerGold(RefundAmount);
            OutGoldSpent -= RefundAmount;

            UE_LOG(LogTemp, Warning, TEXT("ShopComponent: Partial purchase - refunded %d gold"), RefundAmount);
        }

        return true;
    }
    else
    {
        // 실패 시 골드 반환
        ModifyPlayerGold(TotalPrice);
        UE_LOG(LogTemp, Warning, TEXT("ShopComponent: Failed to add item - inventory full or weight exceeded"));
        return false;
    }
}

int32 UShopComponent::GetSellPrice(UItemBase* Item) const
{
    if (!Item) return 0;

    const FShopItemData* ShopItem = ShopInventory.FindByPredicate([&](const FShopItemData& Data)
        {
            return Data.ItemID == Item->ID;
        });

    float Multiplier = ShopItem ? ShopItem->BuyBackMultiplier : 0.5f;

    return FMath::FloorToInt(Item->ItemStatistics.SellValue * Multiplier);
}

int32 UShopComponent::GetBuyPrice(FName ItemID) const
{
    const FShopItemData* ShopItem = ShopInventory.FindByPredicate([&](const FShopItemData& Data)
        {
            return Data.ItemID == ItemID;
        });

    if (!ShopItem) return 0;

    FItemDataRow* ItemData = GetItemData(ItemID);
    if (!ItemData) return 0;

    return FMath::CeilToInt(ItemData->ItemStatistics.SellValue * ShopItem->SellPriceMultiplier);
}

int32 UShopComponent::GetItemStock(FName ItemID) const
{
    if (const int32* Stock = CurrentStock.Find(ItemID))
    {
        return *Stock;
    }
    return 0;
}

bool UShopComponent::CanBuyItem(FName ItemID, int32 Quantity, FText& OutFailReason) const
{
    const FShopItemData* ShopItem = ShopInventory.FindByPredicate([&](const FShopItemData& Data)
        {
            return Data.ItemID == ItemID;
        });

    if (!ShopItem)
    {
        OutFailReason = FText::Format(
            FText::FromString(TEXT("'{0}' is not available in this shop.")),
            FText::FromName(ItemID));
        return false;
    }

    const int32 Stock = GetItemStock(ItemID);

    // 무한 재고
    if (Stock < 0) return true;

    if (Stock < Quantity)
    {
        OutFailReason = FText::Format(
            FText::FromString(TEXT("Not enough stock. Available: {0}, Requested: {1}")),
            Stock, Quantity);
        return false;
    }

    return true;
}

bool UShopComponent::CanSellItem(UItemBase* Item, int32 Quantity, FText& OutFailReason) const
{
    if (!Item)
    {
        OutFailReason = FText::FromString(TEXT("Invalid item."));
        return false;
    }

    // 상점에서 취급하는 아이템인지 확인 (선택사항)
    const FShopItemData* ShopItem = ShopInventory.FindByPredicate([&](const FShopItemData& Data)
        {
            return Data.ItemID == Item->ID;
        });

    // 모든 아이템 구매 가능하도록 설정하려면 이 체크를 제거
    /*
    if (!ShopItem)
    {
        OutFailReason = FText::Format(
            FText::FromString(TEXT("This shop doesn't buy {0}.")),
            Item->TextData.Name);
        return false;
    }
    */

    return true;
}

FItemDataRow* UShopComponent::GetItemData(FName ItemID) const
{
    if (!ItemDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("ShopComponent: ItemDataTable is not set"));
        return nullptr;
    }

    return ItemDataTable->FindRow<FItemDataRow>(ItemID, TEXT("ShopComponent"));
}

void UShopComponent::UpdateStock(FName ItemID, int32 Amount)
{
    if (int32* Stock = CurrentStock.Find(ItemID))
    {
        // 무한 재고가 아닌 경우만 업데이트
        if (*Stock >= 0)
        {
            *Stock += Amount;

            const FShopItemData* ShopData = ShopInventory.FindByPredicate([&](const FShopItemData& Data)
                {
                    return Data.ItemID == ItemID;
                });

            // 최대 재고 제한
            if (ShopData && ShopData->MaxStock > 0)
            {
                *Stock = FMath::Clamp(*Stock, 0, ShopData->MaxStock);
            }

            UE_LOG(LogTemp, Log, TEXT("ShopComponent: Stock updated for %s: %d"),
                *ItemID.ToString(), *Stock);
        }
    }
}

void UShopComponent::RestockItem(FName ItemID)
{
    const FShopItemData* ShopData = ShopInventory.FindByPredicate([&](const FShopItemData& Data)
        {
            return Data.ItemID == ItemID;
        });

    if (ShopData && ShopData->InitialStock >= 0)
    {
        if (int32* Stock = CurrentStock.Find(ItemID))
        {
            *Stock = ShopData->InitialStock;
            OnShopInventoryUpdated.Broadcast();

            UE_LOG(LogTemp, Log, TEXT("ShopComponent: Restocked %s to %d"),
                *ItemID.ToString(), *Stock);
        }
    }
}

bool UShopComponent::ModifyPlayerGold(int32 Amount)
{
    APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

    if (!Player)
    {
        UE_LOG(LogTemp, Error, TEXT("ShopComponent: Player not found"));
        return false;
    }

    // TODO: 플레이어 골드 시스템 연동
    // 예: return Player->GetPlayerStats()->ModifyGold(Amount);

    // 임시 구현 (항상 성공)
    UE_LOG(LogTemp, Log, TEXT("ShopComponent: Gold modified by %d"), Amount);
    return true;
}

UInventoryComponent* UShopComponent::GetPlayerInventory() const
{
    APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(
        UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

    if (Player)
    {
        return Player->GetInventory();
    }

    UE_LOG(LogTemp, Error, TEXT("ShopComponent: Player or inventory not found"));
    return nullptr;
}

UItemBase* UShopComponent::CreateShopItem(FName ItemID, int32 Quantity)
{
    FItemDataRow* ItemData = GetItemData(ItemID);
    if (!ItemData)
    {
        return nullptr;
    }

    UItemBase* NewItem = NewObject<UItemBase>();
    if (!NewItem)
    {
        return nullptr;
    }

    // ItemData에서 정보 복사
    NewItem->ID = ItemData->ID;
    NewItem->ItemType = ItemData->ItemType;
    NewItem->ItemQuality = ItemData->ItemQuality;
    NewItem->NumericData = ItemData->NumericData;
    NewItem->TextData = ItemData->TextData;
    NewItem->AssetData = ItemData->AssetData;
    NewItem->ItemStatistics = ItemData->ItemStatistics;
    NewItem->Quantity = Quantity;

    return NewItem;
}