#pragma once

#include "CoreMinimal.h"
#include "Data/ItemData.h"
#include "ItemBase.generated.h"

class ABaseCharacter_SB;
class UInventoryComponent;

UCLASS()
class STILLBOUND_API UItemBase : public UObject
{
	GENERATED_BODY()
	
public:
	UItemBase();

	UFUNCTION(Category = "Item")
	UItemBase* CreateItemCopy();
	
	UFUNCTION(Category ="Item")
	FORCEINLINE float GetItemStackWeight() const { return Quantity * NumericData.Weight; };

	UFUNCTION(Category = "Item")
	FORCEINLINE float GetItemSingleWeight() const { return NumericData.Weight; };

	UFUNCTION(Category = "Item")
	FORCEINLINE bool IsFullItemStack() const { return Quantity == NumericData.MaxStackSize; };

	UFUNCTION(Category = "Item")
	void SetQuantity(const int32 NewQuantity);

	UFUNCTION(Category = "Item")
	virtual void Use(ABaseCharacter_SB* Character);

public:
	UPROPERTY()
	UInventoryComponent* OwningInventory;

	UPROPERTY(VisibleAnywhere, Category = "Item", meta = (UIMin=1, UIMax=100))
	int32 Quantity;

	UPROPERTY(EditAnywhere, Category = "Item")
	FName ID;

	UPROPERTY(EditAnywhere, Category = "Item")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, Category = "Item")
	EItemQuality ItemQuality;

	UPROPERTY(EditAnywhere, Category = "Item")
	FItemStatistics ItemStatistics;

	UPROPERTY(EditAnywhere, Category = "Item")
	FItemTextData TextData;

	UPROPERTY(EditAnywhere, Category = "Item")
	FItemNumericData NumericData;

	UPROPERTY(EditAnywhere, Category = "Item")
	FItemAssetData AssetData;

	UPROPERTY(EditAnywhere, Category = "Item")
	TSoftClassPtr<AActor> PickupActorClass;

protected:
	bool operator==(const FName& OtherID) const
	{
		return ID == OtherID;
	}
	
};
