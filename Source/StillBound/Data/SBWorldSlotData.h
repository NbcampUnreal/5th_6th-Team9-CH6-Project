// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SBWorldSlotData.generated.h"

/**
 * 
 */
UCLASS()
class STILLBOUND_API USBWorldSlotData : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly) FString SlotId;
    UPROPERTY(BlueprintReadOnly) FString WorldName;
    UPROPERTY(BlueprintReadOnly) FDateTime LastPlayed;
    UPROPERTY(BlueprintReadOnly) int32 Day = 1;
    UPROPERTY(BlueprintReadOnly) FString PlayerName;
    UPROPERTY(BlueprintReadOnly) int32 PlayerLevel = 1;

    static USBWorldSlotData* Make(
        UObject* Outer,
        const FString& InSlotId,
        const FString& InWorldName,
        const FDateTime& InLastPlayed,
        int32 InDay,
        const FString& InPlayerName,
        int32 InPlayerLevel
    );
};