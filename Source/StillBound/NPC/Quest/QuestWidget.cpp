#include "QuestWidget.h"
#include "QuestComponent.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

void UQuestWidget::NativeConstruct()
{
    Super::NativeConstruct();
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

    // 퀘스트 없으면 "없음" 텍스트 표시
    if (TXT_NoQuest)
    {
        TXT_NoQuest->SetVisibility(
            ActiveQuests.Num() == 0 ?
            ESlateVisibility::Visible :
            ESlateVisibility::Collapsed
        );
    }

    for (const FQuestProgress& Progress : ActiveQuests)
    {
        FQuestDataRow* QuestData = QuestComponent->GetQuestData(Progress.QuestID);
        if (!QuestData) continue;

        // 목표 텍스트 동적 생성
        UTextBlock* ObjectiveText = NewObject<UTextBlock>(this);
        if (!ObjectiveText) continue;

        // "나무 수집: 3/5" 형태로 표시
        FString ObjectiveStr;
        if (QuestData->TargetCount > 0)
        {
            ObjectiveStr = FString::Printf(TEXT("? %s (%d/%d)"),
                *QuestData->ObjectiveText.ToString(),
                Progress.CurrentCount,
                QuestData->TargetCount
            );
        }
        else
        {
            ObjectiveStr = FString::Printf(TEXT("? %s"),
                *QuestData->ObjectiveText.ToString()
            );
        }

        // 완료 가능 상태면 색상 변경
        if (Progress.State == EQuestState::Completed)
        {
            ObjectiveText->SetColorAndOpacity(FLinearColor(0.f, 1.f, 0.f, 1.f)); // 초록색
            ObjectiveStr += TEXT(" [완료!]");
        }
        else
        {
            ObjectiveText->SetColorAndOpacity(FLinearColor::White);
        }

        ObjectiveText->SetText(FText::FromString(ObjectiveStr));

        // 폰트 사이즈 설정
        FSlateFontInfo FontInfo = ObjectiveText->GetFont();
        FontInfo.Size = 14;
        ObjectiveText->SetFont(FontInfo);

        VB_QuestObjectives->AddChildToVerticalBox(ObjectiveText);
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