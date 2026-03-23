#include "QuestWidget.h"
#include "QuestComponent.h"
#include "QuestObjectiveSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UQuestWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UQuestWidget::NativeDestruct()
{
    if (QuestComponent)
    {
        QuestComponent->OnQuestUpdated.RemoveAll(this);
        QuestComponent->OnQuestProgressUpdated.RemoveAll(this);
    }
    Super::NativeDestruct();
}

void UQuestWidget::SetQuestComponent(UQuestComponent* InQuestComponent)
{
    if (!InQuestComponent) return;

    // 기존 바인딩 해제
    if (QuestComponent)
    {
        QuestComponent->OnQuestUpdated.RemoveAll(this);
        QuestComponent->OnQuestProgressUpdated.RemoveAll(this);
    }

    QuestComponent = InQuestComponent;

    // 델리게이트 바인딩
    QuestComponent->OnQuestUpdated.AddUniqueDynamic(this, &UQuestWidget::OnQuestUpdated);
    QuestComponent->OnQuestProgressUpdated.AddUniqueDynamic(this, &UQuestWidget::OnQuestProgressUpdated);

    RefreshQuestObjectives();
}

void UQuestWidget::RefreshQuestObjectives()
{
    if (!VB_QuestObjectives || !QuestComponent) return;

    VB_QuestObjectives->ClearChildren();

    TArray<FQuestProgress> ActiveQuests = QuestComponent->GetActiveQuests();

    // 퀘스트 없으면 위젯 숨기기
    if (ActiveQuests.Num() == 0)
    {
        SetVisibility(ESlateVisibility::Collapsed);

        if (TXT_NoQuest)
            TXT_NoQuest->SetVisibility(ESlateVisibility::Visible);
        
        return;
    }
    //퀘스트 있으면 표시
    SetVisibility(ESlateVisibility::Visible);

    if (TXT_NoQuest)
        TXT_NoQuest->SetVisibility(ESlateVisibility::Collapsed);

    for (const FQuestProgress& Progress : ActiveQuests)
    {
        FQuestDataRow* QuestData = QuestComponent->GetQuestData(Progress.QuestID);
        if (!QuestData) continue;

        //QuestObjectiveSlotClass가 없으면 텍스트 폴백
        if (!QuestObjectiveSlotClass)
        {
            UTextBlock* FallbackText = NewObject<UTextBlock>(this);
            if (!FallbackText) continue;

            FString Str = FString::Printf(TEXT("• %s (%d/%d)"),
                *QuestData->ObjectiveText.ToString(),
                Progress.CurrentCount,
                QuestData->TargetCount);

            FLinearColor Color = (Progress.State == EQuestState::Completed) ?
                FLinearColor(0.29f, 0.87f, 0.50f, 1.f) :
                FLinearColor(0.91f, 0.91f, 0.94f, 1.f);

            FallbackText->SetText(FText::FromString(Str));
            FallbackText->SetColorAndOpacity(Color);

            FSlateFontInfo FontInfo = FallbackText->GetFont();
            FontInfo.Size = 12;
            FallbackText->SetFont(FontInfo);

            VB_QuestObjectives->AddChildToVerticalBox(FallbackText);
            continue;
        }
        //서브 위젯 생성
        UQuestObjectiveSlot* QuestSlot = CreateWidget<UQuestObjectiveSlot>(this, QuestObjectiveSlotClass);
        if (!QuestSlot) continue;

        QuestSlot->SetObjectiveData(*QuestData, Progress);

        UVerticalBoxSlot* VBSlot = VB_QuestObjectives->AddChildToVerticalBox(QuestSlot);
        if (VBSlot)
        {
            VBSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
            VBSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
        }
    }
}

void UQuestWidget::OnQuestUpdated(int32 QuestID, EQuestState NewState)
{
    RefreshQuestObjectives();
}

void UQuestWidget::OnQuestProgressUpdated(int32 QuestID, int32 CurrentCount)
{
    RefreshQuestObjectives();
}