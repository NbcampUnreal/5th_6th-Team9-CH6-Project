// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SBWorldSaveGame.generated.h"

UCLASS()
class STILLBOUND_API USBWorldSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, SaveGame)
    FString SlotId;

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

    // World Meta
    UPROPERTY(BlueprintReadOnly, SaveGame)
    int32 Day = 1;

    // Player Attributes
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
};
