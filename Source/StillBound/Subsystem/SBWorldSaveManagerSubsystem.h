// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/SBWorldIndexSaveGame.h"
#include "Data/SBWorldSaveGame.h"
#include "SBWorldSaveManagerSubsystem.generated.h"

class UAbilitySystemComponent;

UCLASS()
class STILLBOUND_API USBWorldSaveManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    TArray<FSBWorldSlotMeta> GetWorldList();

    UFUNCTION(BlueprintCallable)
    bool CreateWorld(const FString& InWorldName, FSBWorldSlotMeta& OutCreated);

    UFUNCTION(BlueprintCallable)
    bool DeleteWorld(const FString& SlotId);

    UFUNCTION(BlueprintCallable)
    void SetCurrentSlotId(const FString& InSlotId) { CurrentSlotId = InSlotId; }

    UFUNCTION(BlueprintCallable)
    FString GetCurrentSlotId() const { return CurrentSlotId; }

    UFUNCTION(BlueprintCallable)
    bool TouchWorldLastPlayed(const FString& SlotId);

    UFUNCTION(BlueprintCallable)
    bool TouchCurrentWorldLastPlayed();

    // World Save / Load
    UFUNCTION(BlueprintCallable)
    bool SaveCurrentWorldFromPawn(APawn* Pawn);

    // 콘솔 로드용
    UFUNCTION(BlueprintCallable)
    bool LoadCurrentWorldToPawn(APawn* Pawn);

    // 스폰 위치 보정용
    UFUNCTION(BlueprintCallable)
    bool LoadCurrentWorldTransformToPawn(APawn* Pawn);

    UFUNCTION(BlueprintCallable)
    bool LoadCurrentWorldAttributesToPawn(APawn* Pawn);

    UFUNCTION(BlueprintCallable)
    bool LoadCurrentWorldInventoryToPawn(APawn* Pawn);

private:
    static const FString IndexSlotName;

    USBWorldIndexSaveGame* LoadOrCreateIndex();
    void SaveIndex(USBWorldIndexSaveGame* Index);
    FString MakeUniqueWorldName(const TArray<FSBWorldSlotMeta>& List, FString BaseName) const;

    USBWorldSaveGame* LoadOrCreateWorldSave(const FString& SlotId);
    bool SaveWorldSave(const FString& SlotId, USBWorldSaveGame* WorldSave);

    // 3단계 helpers
    bool FillPlayerAttributesFromPawn(APawn* Pawn, USBWorldSaveGame* Save);
    bool ApplyPlayerAttributesToPawn(APawn* Pawn, const USBWorldSaveGame* Save);
    void UpdateIndexMetaFromWorldSave(const FString& SlotId, const USBWorldSaveGame* WorldSave);

// Save 관련
private:
    UPROPERTY()
    FString CurrentSlotId;

    bool FillInventoryFromPawn(APawn* Pawn, USBWorldSaveGame* Save);
    bool ApplyInventoryToPawn(APawn* Pawn, const USBWorldSaveGame* Save);

    bool FillPlacedBuildingsFromPawn(APawn* Pawn, USBWorldSaveGame* Save);
    bool ApplyPlacedBuildingsToPawn(APawn* Pawn, const USBWorldSaveGame* Save);

public:
    UFUNCTION(BlueprintCallable)
    bool LoadCurrentWorldBuildingsToPawn(APawn* Pawn);
};
