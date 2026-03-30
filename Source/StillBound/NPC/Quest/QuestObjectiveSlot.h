// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestData.h"
#include "QuestObjectiveSlot.generated.h"

class UTextBlock;
class UProgressBar;
class UBorder;
/**
 *
 */
UCLASS()
class STILLBOUND_API UQuestObjectiveSlot : public UUserWidget
{
    GENERATED_BODY()

public:
    // 퀘스트 데이터 설정
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SetObjectiveData(const FQuestDataRow& QuestData, const FQuestProgress& Progress);

    // 가이드 퀘스트 데이터 설정
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SetGuideQuestData(
        const FText& Title,
        const FText& Desc,
        const FText& Reward
    );

protected:
    // 퀘스트 이름
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_QuestName;

    // 진행 카운트 (2/3)
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_QuestCount;

    // 목표 설명 텍스트
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ObjectiveDesc;

    // 가이드 퀘스트 보상 텍스트
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* TXT_Reward;

    // 진행도 바
    UPROPERTY(meta = (BindWidget))
    UProgressBar* PB_Progress;

    // 완료 배지 Border
    UPROPERTY(meta = (BindWidget))
    UBorder* Border_CompleteBadge;

    // 완료 배지 텍스트
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_CompleteBadge;
};