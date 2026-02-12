// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/SBWorldSaveManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Guid.h"

const FString USBWorldSaveManagerSubsystem::IndexSlotName = TEXT("SB_WorldIndex");

USBWorldIndexSaveGame* USBWorldSaveManagerSubsystem::LoadOrCreateIndex()
{
    if (UGameplayStatics::DoesSaveGameExist(IndexSlotName, 0))
    {
        if (auto* Loaded = Cast<USBWorldIndexSaveGame>(UGameplayStatics::LoadGameFromSlot(IndexSlotName, 0)))
        {
            return Loaded;
        }
    }
    return Cast<USBWorldIndexSaveGame>(UGameplayStatics::CreateSaveGameObject(USBWorldIndexSaveGame::StaticClass()));
}

void USBWorldSaveManagerSubsystem::SaveIndex(USBWorldIndexSaveGame* Index)
{
    if (!Index) return;
    UGameplayStatics::SaveGameToSlot(Index, IndexSlotName, 0);
}

FString USBWorldSaveManagerSubsystem::MakeUniqueWorldName(const TArray<FSBWorldSlotMeta>& List, FString BaseName) const
{
    BaseName = BaseName.TrimStartAndEnd();
    if (BaseName.IsEmpty()) BaseName = TEXT("New World");

    TSet<FString> Existing;
    for (const auto& M : List) Existing.Add(M.WorldName);

    if (!Existing.Contains(BaseName)) return BaseName;

    int32 Suffix = 2;
    while (Existing.Contains(FString::Printf(TEXT("%s (%d)"), *BaseName, Suffix)))
    {
        ++Suffix;
    }
    return FString::Printf(TEXT("%s (%d)"), *BaseName, Suffix);
}

TArray<FSBWorldSlotMeta> USBWorldSaveManagerSubsystem::GetWorldList()
{
    USBWorldIndexSaveGame* Index = LoadOrCreateIndex();
    if (!Index) return {};
    return Index->Worlds;
}

bool USBWorldSaveManagerSubsystem::CreateWorld(const FString& InWorldName, FSBWorldSlotMeta& OutCreated)
{
    USBWorldIndexSaveGame* Index = LoadOrCreateIndex();
    if (!Index) return false;

    FSBWorldSlotMeta NewMeta;
    NewMeta.SlotId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
    NewMeta.WorldName = MakeUniqueWorldName(Index->Worlds, InWorldName);
    NewMeta.LastPlayed = FDateTime::Now();
    NewMeta.Day = 1;
    NewMeta.PlayerName = TEXT("Player");
    NewMeta.PlayerLevel = 1;

    Index->Worlds.Add(NewMeta);
    SaveIndex(Index);

    OutCreated = NewMeta;
    return true;
}

bool USBWorldSaveManagerSubsystem::DeleteWorld(const FString& SlotId)
{
    USBWorldIndexSaveGame* Index = LoadOrCreateIndex();
    if (!Index) return false;

    const int32 Removed = Index->Worlds.RemoveAll([&](const FSBWorldSlotMeta& M)
        {
            return M.SlotId == SlotId;
        });

    if (Removed > 0)
    {
        SaveIndex(Index);
        // (나중에 어떤방식으로 할지 정해지면) 실제 월드 세이브가 SlotId로 저장되어 있다면 여기서 DeleteGameInSlot 해도 된다카는듯
        // UGameplayStatics::DeleteGameInSlot(SlotId, 0);
        return true;
    }
    return false;
}
