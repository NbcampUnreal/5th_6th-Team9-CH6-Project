// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/InteractionInterface.h"
#include "Character/PlayerCharacter_SB.h"
#include "Data/ItemData.h"
#include "DialogueComponent.h"
#include "NPCCharacter.generated.h"

class UDialogueWidget;
class ANPCAIController;
class UInventoryComponent;
class UDialogueComponent;
class UWidgetComponent;
class UShopWidget;
struct FItemDataRow;

UENUM(BlueprintType)
enum class ENPCRegion : uint8
{
	Forest UMETA(DisplayName = "숲 (지역 A)"),
	//Snowfield UMETA(DisplayName = "설산 (지역 B)"),
	//Desert UMETA(DisplayName = "사막 (지역 C)"),
	//Volcano UMETA(DisplayName = "화산 (지역 D)")
};

USTRUCT(BlueprintType)
struct FShopItemData
{
    GENERATED_BODY()

    /** 아이템 Row Name */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemRowName;

    /** 현재 재고 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentStock;

    /** 최대 재고 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxStock;

    // 기본 생성자
    FShopItemData()
        : ItemRowName(NAME_None)
        , CurrentStock(99)
        , MaxStock(99)
    {
    }
};

UCLASS()
class STILLBOUND_API ANPCCharacter : public ACharacter,
    public IInteractionInterface
{
    GENERATED_BODY()

public:
    ANPCCharacter();

    // === 인터페이스 구현 ===
    virtual void BeginFocus_Implementation() override;
    virtual void EndFocus_Implementation() override;
    virtual void EndInteract_Implementation() override;
    virtual void Interact_Implementation(APlayerCharacter_SB* PlayerCharacter) override;
    virtual FInteractableData GetInteractableData_Implementation() override;
    virtual float GetInteractionDistance_Implementation() override;

    UFUNCTION(BlueprintCallable, Category = "Shop")
    void OpenShop();

    // === 대화 컴포넌트 ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    UDialogueComponent* DialogueComponent;

    // === Widget ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UDialogueWidget> DialogueWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    FText ShopName;

    // 최대 판매 아이템 수 (랜덤 선택)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    int32 MaxShopItems = 10;

    // Low-tier만 판매 (Weapon, Tool)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    bool bOnlyLowTierEquipment = true;

    //  Low-tier 키워드 (이름에 포함되면 Low-tier로 판단)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    TArray<FString> LowTierKeywords;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    float PriceMultiplier = 1.0f;  // 가격 배율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    UDataTable* ItemDataTable;

    UFUNCTION(BlueprintCallable, Category = "Shop")
    bool SellItemToPlayer(APlayerCharacter_SB* Player, FName ItemRowName, int32 Quantity = 1);

    UFUNCTION(BlueprintPure, Category = "Shop")
    int32 GetItemPrice(FName ItemRowName) const;

    FItemDataRow* GetItemData(FName ItemRowName) const;

    // 플레이어로부터 아이템 구매 (새로 추가)
    UFUNCTION(BlueprintCallable, Category = "Shop")
    bool BuyItemFromPlayer(APlayerCharacter_SB* Player, UItemBase* Item, int32 Quantity = 1);

    // 상점 초기화
    void InitializeShopItems();

    // NPC 상점 아이템 리스트 가져오기
    UFUNCTION(BlueprintPure, Category = "Shop")
    const TArray<FShopItemData>& GetShopItemList() const { return ShopItemList; }

    UFUNCTION(BlueprintCallable)
    void ReduceShopItemStock(FName ItemRowName, int32 Amount);


protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === 내부 함수 ===
    void MonitorStateChanges();

    ANPCAIController* GetNPCAIController() const;

    // === 대화 이벤트 핸들러 ===
    UFUNCTION()
    void OnDialogueStart(const FDialogueRow& DialogueData);

    UFUNCTION()
    void OnDialogueUpdate(const FDialogueRow& DialogueData);

    UFUNCTION()
    void OnDialogueEnd();

    UFUNCTION()
    void OnOptionSelected(int32 OptionIndex);

    // === 블루프린트 이벤트 ===
    UFUNCTION(BlueprintImplementableEvent, Category = "NPC")
    void OnPlayerDetected(AActor* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC")
    void OnPlayerLost();

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC")
    void OnInteractionStarted(AActor* Interactor);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC")
    void OnInteractionEnded(AActor* Interactor);

    // === 상태 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName = TEXT("NPC");

    UPROPERTY(BlueprintReadOnly, Category = "NPC")
    bool bIsInteracting = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC")
    AActor* CurrentInteractor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    TSubclassOf<UShopWidget> ShopWidgetClass;

    UPROPERTY(VisibleAnywhere, Category = "Shop")
    TArray<FShopItemData> ShopItemList;

private:
    uint8 LastNPCState = 0;

    // Widget 인스턴스
    UPROPERTY()
    UDialogueWidget* DialogueWidget = nullptr;

    UPROPERTY()
    UShopWidget* ShopWidget = nullptr;

    // 실제 판매할 아이템 ID 목록 (BeginPlay에서 자동 생성)
    TArray<FName> SellableItemIDs;

    // Low-tier 아이템인지 판단
    bool IsLowTierItem(const FItemDataRow* ItemData) const;

    bool IsLowTierItem_ByRowName(FName RowName, EItemType ItemType) const;

};
