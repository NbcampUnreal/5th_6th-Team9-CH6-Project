// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/SBWorldIndexSaveGame.h"
#include "SBWorldSaveManagerSubsystem.generated.h"

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

private:
    static const FString IndexSlotName;

    USBWorldIndexSaveGame* LoadOrCreateIndex();
    void SaveIndex(USBWorldIndexSaveGame* Index);
    FString MakeUniqueWorldName(const TArray<FSBWorldSlotMeta>& List, FString BaseName) const;

    UPROPERTY()
    FString CurrentSlotId;
};