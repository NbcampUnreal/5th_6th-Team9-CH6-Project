// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Quest/QuestObjectiveSlot.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"

void UQuestObjectiveSlot::SetObjectiveData(const FQuestDataRow& QuestData, const FQuestProgress& Progress)
{
    //if (!QuestData) return;

    bool bCompleted = (Progress.State == EQuestState::Completed);

    // 퀘스트 이름
    if (TXT_QuestName)
    {
        TXT_QuestName->SetText(QuestData.QuestName);

        // 완료 시 초록색
        FLinearColor NameColor = bCompleted ?
            FLinearColor(0.29f, 0.87f, 0.50f, 1.f) :
            FLinearColor(0.91f, 0.91f, 0.94f, 1.f);
        TXT_QuestName->SetColorAndOpacity(NameColor);
    }

    // 진행 카운트 (2/3)
    if (TXT_QuestCount)
    {
        FString CountStr = FString::Printf(TEXT("%d / %d"),
            Progress.CurrentCount,
            QuestData.TargetCount);
        TXT_QuestCount->SetText(FText::FromString(CountStr));

        FLinearColor CountColor = bCompleted ?
            FLinearColor(0.29f, 0.87f, 0.50f, 1.f) :
            FLinearColor(0.29f, 0.62f, 1.0f, 1.f);
        TXT_QuestCount->SetColorAndOpacity(CountColor);
    }

    // 목표 설명
    if (TXT_ObjectiveDesc)
    {
        TXT_ObjectiveDesc->SetText(QuestData.ObjectiveText);

        FLinearColor DescColor = bCompleted ?
            FLinearColor(0.53f, 0.87f, 0.67f, 1.f) :
            FLinearColor(0.53f, 0.53f, 0.67f, 1.f);
        TXT_ObjectiveDesc->SetColorAndOpacity(DescColor);
    }

    // 진행도 바
    if (PB_Progress && QuestData.TargetCount > 0)
    {
        float Percent = (float)Progress.CurrentCount / (float)QuestData.TargetCount;
        Percent = FMath::Clamp(Percent, 0.f, 1.f);
        PB_Progress->SetPercent(Percent);

        // 완료 시
        FLinearColor BarColor = bCompleted ?
            FLinearColor(0.29f, 0.87f, 0.50f, 1.f) :
            FLinearColor(0.29f, 0.62f, 1.0f, 1.f);
        PB_Progress->SetFillColorAndOpacity(BarColor);
    }

    // 완료 배지
    if (Border_CompleteBadge)
    {
        Border_CompleteBadge->SetVisibility(
            bCompleted ?
            ESlateVisibility::Visible :
            ESlateVisibility::Collapsed
        );
    }
}

