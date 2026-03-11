#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CraftingRecipeRow.generated.h"

USTRUCT(BlueprintType)
struct FRecipeIngredient
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName ItemID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Count = 1;
};

USTRUCT(BlueprintType)
struct FCraftingRecipeRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere) 
    FName ResultItemID = NAME_None;

    UPROPERTY(EditAnywhere) 
    int32 ResultCount = 1;

    UPROPERTY(EditAnywhere) 
    TArray<FRecipeIngredient> Ingredients;

    UPROPERTY(EditAnywhere) 
    FName RequiredStationTag = NAME_None;
};
