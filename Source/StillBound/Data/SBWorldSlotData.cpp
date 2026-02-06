// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/SBWorldSlotData.h"

USBWorldSlotData* USBWorldSlotData::Make(
    UObject* Outer,
    const FString& InSlotId,
    const FString& InWorldName,
    const FDateTime& InLastPlayed,
    int32 InDay,
    const FString& InPlayerName,
    int32 InPlayerLevel
) {
    USBWorldSlotData* Obj = NewObject<USBWorldSlotData>(Outer);
    Obj->SlotId = InSlotId;
    Obj->WorldName = InWorldName;
    Obj->LastPlayed = InLastPlayed;
    Obj->Day = InDay;
    Obj->PlayerName = InPlayerName;
    Obj->PlayerLevel = InPlayerLevel;
    return Obj;
}