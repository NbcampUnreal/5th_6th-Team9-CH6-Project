// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DialogueData.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EDialogueType : uint8
{
    Normal      UMETA(DisplayName = "Normal"),
    Quest       UMETA(DisplayName = "Quest"),
    Trade       UMETA(DisplayName = "Trade"),
    Information UMETA(DisplayName = "Information")
};

UENUM(BlueprintType)
enum class EMenuType : uint8
{
    None        UMETA(DisplayName = "None"),
    Dialogue    UMETA(DisplayName = "Dialogue"),
    Quest       UMETA(DisplayName = "Quest"),
    Trade       UMETA(DisplayName = "Trade"),
    Exit        UMETA(DisplayName = "EXIT")
};

UENUM(BlueprintType)
enum class EConditionType : uint8
{
    None            UMETA(DisplayName = "조건 없음"),
    HasItem         UMETA(DisplayName = "아이템 보유"),
    HasGold         UMETA(DisplayName = "골드 보유"),
    QuestCompleted  UMETA(DisplayName = "퀘스트 완료"),
    QuestActive     UMETA(DisplayName = "퀘스트 진행 중")
};

USTRUCT(BlueprintType)
struct FDialogueCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    EConditionType ConditionType = EConditionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    FString ConditionValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    int32 RequiredAmount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    bool bInvert = false;

    FDialogueCondition()
        : ConditionType(EConditionType::None)
        , ConditionValue("")
        , RequiredAmount(1)
        , bInvert(false)
    {
    }
};

USTRUCT(BlueprintType)
struct FDialogueOption
{
    GENERATED_BODY()

    // 선택지 텍스트
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText OptionText;

    // 다음 대화 ID (0이면 대화 종료)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NextDialogueID = 0;

    // 메뉴 전환 (다른 테이블로 이동)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMenuType SwitchToMenu = EMenuType::None;

    // 조건
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDialogueCondition RequiredCondition;

    FDialogueOption()
        : OptionText(FText::FromString(TEXT("계속")))
        , NextDialogueID(0)
        , SwitchToMenu(EMenuType::None)
    {
    }
};

/**
 * 대화 데이터 (DataTable Row)
 */
USTRUCT(BlueprintType)
struct FDialogueRow : public FTableRowBase
{
    GENERATED_BODY()

    // 대화 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DialogueID = 0;

    // NPC 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText NPCName;

    // 대화 내용
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DialogueText;

    // 대화 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDialogueType DialogueType = EDialogueType::Normal;

    // 선택지 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDialogueOption> Options;

    // NPC 초상화 (선택사항)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* NPCPortrait = nullptr;

    FDialogueRow()
    {
    }
};
