#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/InventoryTypes.h"
#include "InventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_MULTICAST_DELEGATE(FOnHotbarUpdated);

class UItemBase;

UENUM(BlueprintType)
enum class EItemAddResult : uint8
{
    IAR_NoItemAdded UMETA(DisplayName = "No item added"),
    IAR_PartialAmountItemAdded UMETA(DisplayName = "Partial amount of item added"),
    IAR_AllItemAdded UMETA(DisplayName = "All of item added")
};

USTRUCT(BlueprintType)
struct FItemAddResult
{
    GENERATED_BODY()

    FItemAddResult() :
        ActualAmountAdded(0),
        OperationResult(EItemAddResult::IAR_NoItemAdded),
        ResultMessage(FText::GetEmpty())
    {};

    // Actual amount of item that was added to the inventory
    UPROPERTY(BlueprintReadOnly, Category="Item Add Result")
    int32 ActualAmountAdded;

    // Enum representing the end state of an add item operation
    UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
    EItemAddResult OperationResult;

    // Informational message that can be passed with the result
    UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
    FText ResultMessage;

    static FItemAddResult AddedNone(const FText& ErrorText)
    {
        FItemAddResult AddedNoneResult;
        AddedNoneResult.ActualAmountAdded = 0;
        AddedNoneResult.OperationResult = EItemAddResult::IAR_NoItemAdded;
        AddedNoneResult.ResultMessage = ErrorText;
        return AddedNoneResult;

    };

    static FItemAddResult AddedPartial(const int32 PartialAmountAdded, const FText& ErrorText)
    {
        FItemAddResult AddedPartialResult;
        AddedPartialResult.ActualAmountAdded = PartialAmountAdded;
        AddedPartialResult.OperationResult = EItemAddResult::IAR_PartialAmountItemAdded;
        AddedPartialResult.ResultMessage = ErrorText;
        return AddedPartialResult;

    };
    static FItemAddResult AddedAll(const int32 AmountAdded, const FText& Message)
    {
        FItemAddResult AddedAllResult;
        AddedAllResult.ActualAmountAdded = AmountAdded;
        AddedAllResult.OperationResult = EItemAddResult::IAR_AllItemAdded;
        AddedAllResult.ResultMessage = Message;
        return AddedAllResult;
    };
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STILLBOUND_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ///===============================================================================
    /// PROPERTIES & VARIABLES
    ///===============================================================================
    FOnInventoryUpdated OnInventoryUpdated;
    FOnHotbarUpdated OnHotbarUpdated;

    ///===============================================================================
    /// FUNCTIONS
    ///===============================================================================
    UInventoryComponent();

    UFUNCTION(Category = "Inventory")
    FItemAddResult HandleAddItem(UItemBase* InputItem);

    UFUNCTION(Category = "Inventory")
    UItemBase* FindMatchingItem(UItemBase* ItemIn) const;
    UFUNCTION(Category = "Inventory")
    UItemBase* FindNextItemByID(UItemBase* ItemIn) const;
    UFUNCTION(Category = "Inventory")
    UItemBase* FindNextPartialStack(UItemBase* ItemIn) const;

    UFUNCTION(Category = "Inventory")
    UItemBase* FindNextPartialStackInHotbar(UItemBase* ItemIn) const;


    UFUNCTION(Category = "Inventory")
    int32 FindFirstEmptyInventorySlot() const;

    UFUNCTION(Category = "Inventory")
    int32 FindFirstEmptyHotbarIndex() const;

    UFUNCTION(Category = "Inventory")
    void RemoveSingleInstanceOfItem(UItemBase* ItemToRemove);

    UFUNCTION(Category = "Inventory")
    int32 RemoveAmountOfItem(UItemBase* ItemIn, int32 DesiredAmountToRemove);

    UFUNCTION(Category = "Inventory")
    void SplitExistingStack(UItemBase* ItemIn, const int32 AmountToSplit);

    UFUNCTION(Category = "Inventory")
    FItemAddResult HandleAddItem_AutoHotbarFirst(UItemBase* InputItem);

    UFUNCTION(Category = "Inventory")
    int32 GetOccupiedSlotCount() const;

    UItemBase* GetItemAtIndex(int32 Index) const;
    int32 RemoveAmountAtIndex(int32 Index, int32 Quantity);

    UItemBase* GetItemInContainer(ESlotContainer InContainer, int32 Index) const;
    int32 RemoveAmountInContainer(ESlotContainer InContainer, int32 Index, int32 Quantity);
    /// getters
    UFUNCTION(Category = "Inventory")
    FORCEINLINE float GetInventoryTotalWeight() const { return InventoryTotalWeight; };

    UFUNCTION(Category = "Inventory")
    FORCEINLINE float GetWeightCapacity() const { return InventoryWeightCapacity; };

    UFUNCTION(Category = "Inventory")
    FORCEINLINE int32 GetSlotCapacity() const { return InventorySlotsCapacity; };

    FORCEINLINE const TArray<TObjectPtr<UItemBase>>& GetInventorySlots() const { return InventorySlots; };

    UFUNCTION(Category = "Horbar")
    FORCEINLINE int32 GetHotbarCapacity() const { return HotbarSlotsCapacity; };

    FORCEINLINE const TArray<TObjectPtr<UItemBase>>& GetHotbarSlots() const { return HotbarContents; };

    bool MoveSlotItem(ESlotContainer FromContainer, int32 FromIndex, ESlotContainer ToContainer, int32 ToIndex, bool bAllowSwap);


    /// setters
    UFUNCTION(Category = "Inventory")
    FORCEINLINE void SetSlotsCapacity(const int32 NewSlotsCapacity) { InventorySlotsCapacity = NewSlotsCapacity; };

    UFUNCTION(Category = "Inventory")
    FORCEINLINE void SetWeightCapacity(const float NewWeightCapacity) { InventoryWeightCapacity = NewWeightCapacity; };

protected:
    ///===============================================================================
    /// PROPERTIES & VARIABLES
    ///===============================================================================
    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    float InventoryTotalWeight;

    UPROPERTY(EditInstanceOnly, Category = "Inventory")
    int32 InventorySlotsCapacity;

    UPROPERTY(EditInstanceOnly, Category = "Inventory")
    float InventoryWeightCapacity;

    UPROPERTY()
    TArray<TObjectPtr<UItemBase>> InventorySlots;

    UPROPERTY(EditInstanceOnly, Category="Hotbar")
    int32 HotbarSlotsCapacity = 8;

    UPROPERTY(EditInstanceOnly, Category = "Hotbar")
    TArray<TObjectPtr<UItemBase>> HotbarContents;

    ///===============================================================================
    /// FUNCTIONS
    ///===============================================================================
    virtual void BeginPlay() override;

    FItemAddResult HandleNonStackableItems(UItemBase* InputItem);
    int32 HandleStackableItems(UItemBase* ItemIn, int32 RequestedAddAmount);
    int32 CalculateWeightAddAmount(UItemBase* ItemIn, int32 RequestedAddAmount);
    int32 CalculateNumberForFullStack(UItemBase* StackableItem, int32 InitialRequestedAddAmount);

    void AddNewItem(UItemBase* Item, const int32 AmountToAdd);

};
