#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "QuestData.generated.h"

// ����Ʈ Ÿ��
UENUM(BlueprintType)
enum class EQuestType : uint8
{
    None        UMETA(DisplayName = "None"),
    Collect     UMETA(DisplayName = "Collect"),
    Talk        UMETA(DisplayName = "Dialogue"),
};

// ����Ʈ ����
UENUM(BlueprintType)
enum class EQuestState : uint8
{
    None        UMETA(DisplayName = "����"),
    Available   UMETA(DisplayName = "���� ����"),
    Active      UMETA(DisplayName = "���� ��"),
    Completed   UMETA(DisplayName = "�Ϸ� ����"),
    Rewarded    UMETA(DisplayName = "���� ���� �Ϸ�"),
};

// DataTable �� ����ü
USTRUCT(BlueprintType)
struct FQuestDataRow : public FTableRowBase
{
    GENERATED_BODY()

    // ����Ʈ ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 QuestID = 0;

    // ����Ʈ �̸�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestName = FText::GetEmpty();;

    // ����Ʈ ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description = FText::FromString(TEXT("QuestDescription"));

    // ���� �� ǥ�� �ؽ�Ʈ (��ǥ UI�� ǥ��)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText ObjectiveText= FText ::GetEmpty();

    // ����Ʈ Ÿ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestType QuestType = EQuestType::None;

    // ��ǥ ������ ID (���� ����Ʈ)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName TargetItemID = NAME_None;

    // ��ǥ ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 TargetCount = 0;

    // ���� ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RewardGold = 0;

    // ���� ����Ʈ ID (���� ����Ʈ, 0�̸� ����)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 NextQuestID = 0;

    // ���� �� NPC ��ȭ ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 AcceptDialogueID = 0;

    // �Ϸ� �� NPC ��ȭ ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CompleteDialogueID = 0;
};

// �÷��̾ ������ ����Ʈ ���� ����
USTRUCT(BlueprintType)
struct FQuestProgress
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 QuestID = 0;

    UPROPERTY(BlueprintReadWrite)
    EQuestState State = EQuestState::None;

    UPROPERTY(BlueprintReadWrite)
    int32 CurrentCount = 0;  // ���� ����/ų ����
};