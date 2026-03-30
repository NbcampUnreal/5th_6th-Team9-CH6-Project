#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UObject/SoftObjectPath.h"
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
/// Guide Quest Save Data (추가)
/// ===============================================================================

USTRUCT(BlueprintType)
struct FSBGuideQuestSaveData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 SaveVersion = 1;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	bool bHasActiveQuest = false;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	FName ActiveQuestId = NAME_None;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	TArray<int32> ActiveObjectiveCounts;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	TArray<FName> CompletedQuestIds;
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

    // --- 부활 위치 저장 관련 ---
    //부활 위치가 실제로 저장된 적이 있는지 체크
    UPROPERTY(BlueprintReadOnly, SaveGame)
    bool bHasRespawnTransform = false;
    
    // 위치. 회전, 스케일 저장
    UPROPERTY(BlueprintReadOnly, SaveGame)
    FTransform SavedRespawnTransform = FTransform::Identity;
    // ===============

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

    /// ===================================================================
    /// Guide Quest Save Data (클래스 내부 추가)
    /// ===================================================================

    UPROPERTY(BlueprintReadOnly, SaveGame)
    bool bHasGuideQuest = false;

    UPROPERTY(BlueprintReadOnly, SaveGame)
    FSBGuideQuestSaveData SavedGuideQuest;
};