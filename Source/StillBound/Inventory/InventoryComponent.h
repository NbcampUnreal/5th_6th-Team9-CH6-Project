#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

struct FItemDataRow;

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemID = NAME_None;

    UPROPERTY(EditAnywhere, BluePrintReadOnly)
	int32 Quantity = 0;

	bool IsEmpty() const { return ItemID.IsNone() || Quantity <= 0; }
    void Clear()
    {
        ItemID = NAME_None;
        Quantity = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventorySlotUpdated, int32, SlotIndex);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STILLBOUND_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    /** UI 바인딩용 이벤트 */
    UPROPERTY(BlueprintAssignable)
    FOnInventoryUpdated OnInventoryUpdated;

    UFUNCTION(BlueprintCallable)
    void BroadcastInventoryUpdated() const;

    UPROPERTY(BlueprintAssignable)
    FOnInventorySlotUpdated OnInventorySlotUpdated;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite)
    TObjectPtr<UDataTable> ItemDataTable;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void InitializeSlots(int32 InSlotCount);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(FName ItemID, int32 Amount, int32& OutRemaining);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(FName ItemID, int32 Amount, int32& OutRemaining);

protected:
    virtual void BeginPlay() override;

private:
    /** 고정 슬롯 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    TArray<FInventorySlot> Slots;

    /** 기본 슬롯 수 */
    UPROPERTY(EditAnywhere, Category = "Inventory|Config")
    int32 Capacity;

    FInventorySlot* FindStackableSlot(FName ItemID, const FItemDataRow* ItemData);
    FInventorySlot* FindEmptySlot();

    /** 아이템별 MaxStack 조회: 지금은 더미로 처리, 나중에 DataTable 연동 */
    int32 GetMaxStackForItem(FName ItemID) const;

 

};
