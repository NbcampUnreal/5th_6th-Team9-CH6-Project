#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SBWorldSaveGame.generated.h"

/// ===============================================================================
/// SAVE DATA STRUCTURES (클래스 밖, 위쪽에 배치)
/// ===============================================================================

USTRUCT(BlueprintType)
struct FSBItemSlotSaveData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, SaveGame)
    FName ItemID = NAME_None;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 Quantity = 0;

    UPROPERTY(BlueprintReadWrite, SaveGame)
    int32 Index = 0;
};

USTRUCT(BlueprintType)
struct FSBPlacedBuildingSaveData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, SaveGame) FName BuildingID = NAME_None;
    UPROPERTY(BlueprintReadOnly, SaveGame) FTransform Transform;
};

USTRUCT(BlueprintType)
struct FSBWorldDroppedItemSaveData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, SaveGame)
    FName ItemID = NAME_None;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    int32 Quantity = 0;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    FTransform Transform;
};

/// ===============================================================================
/// SAVE GAME CLASS
/// ===============================================================================

UCLASS()
class STILLBOUND_API USBWorldSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, SaveGame)
    FString SlotId;

    // --- Player Transform ---
    UPROPERTY(BlueprintReadOnly, SaveGame)
    bool bHasPlayerTransform = false;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    FVector PlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    FRotator PlayerRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    bool bHasControlRotation = false;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    FRotator SavedControlRotation = FRotator::ZeroRotator;

    // --- World Meta ---
    UPROPERTY(BlueprintReadOnly, SaveGame)
    int32 Day = 1;

    // --- Player Attributes ---
    UPROPERTY(BlueprintReadOnly, SaveGame)
    bool bHasPlayerAttributes = false;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    float SavedHealth = 0.f;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    float SavedMaxHealth = 0.f;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    float SavedStamina = 0.f;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    float SavedMaxStamina = 0.f;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    float SavedLevel = 1.f;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    float SavedExperience = 0.f;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    float SavedAttack = 0.f;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    float SavedDefense = 0.f;

    // --- Inventory & Hotbar  ---
    UPROPERTY(BlueprintReadOnly, SaveGame)
    bool bHasInventory = false;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    TArray<FSBItemSlotSaveData> SavedInventorySlots;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    TArray<FSBItemSlotSaveData> SavedHotbarSlots;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    int32 SavedSelectedHotbarIndex = 0;

    // --- Gold --- //
    UPROPERTY(BlueprintReadOnly, SaveGame)
    int32 SavedGold = 0;

    // --- Buildings  ---
    UPROPERTY(BlueprintReadOnly, SaveGame)
    bool bHasPlacedBuildings = false;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    TArray<FSBPlacedBuildingSaveData> SavedBuildings;

    // --- Drop Items ---
    UPROPERTY(BlueprintReadOnly, SaveGame)
    bool bHasDroppedItems = false;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    TArray<FSBWorldDroppedItemSaveData> SavedDroppedItems;


};