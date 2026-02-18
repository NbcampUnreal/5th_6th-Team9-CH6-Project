// ShopComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ItemData.h"
#include "ShopComponent.generated.h"

class UItemBase;
class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopInventoryUpdated);

/**
 * 상점 아이템 데이터
 */
USTRUCT(BlueprintType)
struct FShopItemData
{
    GENERATED_BODY()

    // 아이템 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemID;

    // 판매 가격 배율 (플레이어가 NPC에게서 살 때)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SellPriceMultiplier = 1.0f;

    // 구매 가격 배율 (플레이어가 NPC에게 팔 때)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BuyBackMultiplier = 0.5f;

    // 초기 재고 (-1이면 무한)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 InitialStock = -1;

    // 최대 재고 (-1이면 무한)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxStock = -1;

    // 재고 재생성 시간 (초, 0이면 재생성 안 함)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RestockTime = 0.0f;

    FShopItemData()
        : ItemID(NAME_None)
        , SellPriceMultiplier(1.0f)
        , BuyBackMultiplier(0.5f)
        , InitialStock(-1)
        , MaxStock(-1)
        , RestockTime(0.0f)
    {
    }
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class STILLBOUND_API UShopComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UShopComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // ========== 상점 설정 ==========

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    FText ShopName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    TArray<FShopItemData> ShopInventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    UDataTable* ItemDataTable;

    // NPC도 자체 인벤토리를 가짐 (구매한 아이템 보관)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    bool bUseNPCInventory = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop", meta = (EditCondition = "bUseNPCInventory"))
    int32 NPCInventorySlots = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop", meta = (EditCondition = "bUseNPCInventory"))
    float NPCInventoryWeightCapacity = 200.0f;

    // ========== 거래 함수 ==========

    // 플레이어가 NPC에게 아이템 판매
    UFUNCTION(BlueprintCallable, Category = "Shop")
    bool SellItemToShop(UItemBase* Item, int32 Quantity, int32& OutGoldReceived);

    // 플레이어가 NPC에게서 아이템 구매
    UFUNCTION(BlueprintCallable, Category = "Shop")
    bool BuyItemFromShop(FName ItemID, int32 Quantity, int32& OutGoldSpent);

    // ========== 가격 계산 ==========

    UFUNCTION(BlueprintPure, Category = "Shop")
    int32 GetSellPrice(UItemBase* Item) const;

    UFUNCTION(BlueprintPure, Category = "Shop")
    int32 GetBuyPrice(FName ItemID) const;

    // ========== 재고 관리 ==========

    UFUNCTION(BlueprintPure, Category = "Shop")
    int32 GetItemStock(FName ItemID) const;

    UFUNCTION(BlueprintPure, Category = "Shop")
    bool CanBuyItem(FName ItemID, int32 Quantity, FText& OutFailReason) const;

    UFUNCTION(BlueprintPure, Category = "Shop")
    bool CanSellItem(UItemBase* Item, int32 Quantity, FText& OutFailReason) const;

    UFUNCTION(BlueprintPure, Category = "Shop")
    TArray<FShopItemData> GetShopInventory() const { return ShopInventory; }

    // NPC 인벤토리 가져오기
    UFUNCTION(BlueprintPure, Category = "Shop")
    UInventoryComponent* GetNPCInventory() const { return NPCInventory; }

    // ========== 이벤트 ==========

    UPROPERTY(BlueprintAssignable, Category = "Shop")
    FOnShopInventoryUpdated OnShopInventoryUpdated;

private:
    // NPC 인벤토리
    UPROPERTY()
    UInventoryComponent* NPCInventory = nullptr;

    // 현재 재고 (런타임)
    TMap<FName, int32> CurrentStock;

    // 재고 재생성 타이머
    TMap<FName, float> RestockTimers;

    // 아이템 데이터 로드
    FItemDataRow* GetItemData(FName ItemID) const;

    // 재고 업데이트
    void UpdateStock(FName ItemID, int32 Amount);

    // 재고 재생성
    void RestockItem(FName ItemID);

    // 플레이어 골드 지급/차감
    bool ModifyPlayerGold(int32 Amount);

    // 플레이어 인벤토리 가져오기
    UInventoryComponent* GetPlayerInventory() const;

    // 상점용 아이템 생성
    UItemBase* CreateShopItem(FName ItemID, int32 Quantity);
};