#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HotbarPanel.generated.h"

class UUniformGridPanel;
class UInventoryItemSlot;
class APlayerCharacter_SB;
class UInventoryComponent;

UCLASS()
class STILLBOUND_API UHotbarPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void RefreshHotbar();

	void InitWithInventory(UInventoryComponent* InInv);

protected:
	virtual void NativeOnInitialized() override;

	void BuildHotbar();

	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* HotbarGrid;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventoryItemSlot> HotbarSlotClass;

	UPROPERTY()
	TArray<TObjectPtr<UInventoryItemSlot>> HotbarSlotWidgets;

	UPROPERTY()
	APlayerCharacter_SB* PlayerCharacter;

	UPROPERTY()
	UInventoryComponent* InventoryReference;

private:
	static constexpr int32 HotbarSize = 8;

};
