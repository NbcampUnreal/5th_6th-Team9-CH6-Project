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

    // 조건 (나중에 확장용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RequiredCondition;

    FDialogueOption()
        : OptionText(FText::FromString(TEXT("계속")))
        , NextDialogueID(0)
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
        /*기본 선택지 추가
        FDialogueOption DefaultOption;
        DefaultOption.OptionText = FText::FromString(TEXT("플레이어가 선택할 답변"));
        DefaultOption.NextDialogueID = 0; // 대화 종료
        Options.Add(DefaultOption);
        */
    }
};
