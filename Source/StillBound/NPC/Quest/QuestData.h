#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "QuestData.generated.h"

// 퀘스트 타입
UENUM(BlueprintType)
enum class EQuestType : uint8
{
    None        UMETA(DisplayName = "None"),
    Collect     UMETA(DisplayName = "Collect"),
    Talk        UMETA(DisplayName = "Dialogue"),
};

// 퀘스트 상태
UENUM(BlueprintType)
enum class EQuestState : uint8
{
    None        UMETA(DisplayName = "없음"),
    Available   UMETA(DisplayName = "수락 가능"),
    Active      UMETA(DisplayName = "진행 중"),
    Completed   UMETA(DisplayName = "완료 가능"),
    Rewarded    UMETA(DisplayName = "보상 수령 완료"),
};

// DataTable 행 구조체
USTRUCT(BlueprintType)
struct FQuestDataRow : public FTableRowBase
{
    GENERATED_BODY()

    // 퀘스트 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 QuestID = 0;

    // 퀘스트 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestName = FText::GetEmpty();

    // 퀘스트 설명
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description = FText::GetEmpty();

    // 진행 중 표시 텍스트 (목표 UI에 표시)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText ObjectiveText= FText ::GetEmpty();

    // 퀘스트 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestType QuestType = EQuestType::None;

    // 목표 아이템 ID (수집 퀘스트)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName TargetItemID = NAME_None;

    // 목표 수량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 TargetCount = 0;

    // 보상 골드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RewardGold = 0;

    // 다음 퀘스트 ID (연계 퀘스트, 0이면 없음)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 NextQuestID = 0;

    // 수락 시 NPC 대화 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 AcceptDialogueID = 0;

    // 완료 시 NPC 대화 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CompleteDialogueID = 0;
};

// 플레이어가 보유한 퀘스트 진행 정보
USTRUCT(BlueprintType)
struct FQuestProgress
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 QuestID = 0;

    UPROPERTY(BlueprintReadWrite)
    EQuestState State = EQuestState::None;

    UPROPERTY(BlueprintReadWrite)
    int32 CurrentCount = 0;  // 현재 수집/킬 수량
};