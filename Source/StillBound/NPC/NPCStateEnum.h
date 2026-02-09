// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "NPCStateEnum.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ENPCMode : uint8
{
    Idle            UMETA(DisplayName = "대기"),
    Alert           UMETA(DisplayName = "경계 (플레이어 감지)"),
    Interacting     UMETA(DisplayName = "상호작용 중"),
    Working         UMETA(DisplayName = "작업 중")
};

