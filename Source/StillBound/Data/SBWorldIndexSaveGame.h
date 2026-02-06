#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SBWorldIndexSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FSBWorldSlotMeta
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, SaveGame) FString SlotId;
    UPROPERTY(BlueprintReadOnly, SaveGame) FString WorldName;
    UPROPERTY(BlueprintReadOnly, SaveGame) FDateTime LastPlayed;
    UPROPERTY(BlueprintReadOnly, SaveGame) int32 Day = 1;
    UPROPERTY(BlueprintReadOnly, SaveGame) FString PlayerName;
    UPROPERTY(BlueprintReadOnly, SaveGame) int32 PlayerLevel = 1;
};

UCLASS()
class STILLBOUND_API USBWorldIndexSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, SaveGame)
    TArray<FSBWorldSlotMeta> Worlds;
};
