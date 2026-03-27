#include "QuestWidget.h"
#include "QuestComponent.h"
#include "QuestObjectiveSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GuideQuest/GuideQuestSubsystem.h"

void UQuestWidget::NativeConstruct()
{
    Super::NativeConstruct();

    //디버깅용 코드
    UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] NativeConstruct called"));

    // 가이드 퀘스트 GuideQuestSubsystem 가져오고 델리게이트 바인딩
    if (UGameInstance* GI = GetGameInstance())
    {
        GuideQuestSubsystem = GI->GetSubsystem<UGuideQuestSubsystem>();
        if (GuideQuestSubsystem)
        {
            GuideQuestSubsystem->OnGuideQuestUpdated.AddDynamic(this, &UQuestWidget::OnGuideQuestUpdated);

            //디버깅용 코드
            UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] GuideQuestSubsystem bound"));
        }
        else
        {
            //디버깅용 코드
            UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] GuideQuestSubsystem is null"));
        }
    }
    else
    {
        //디버깅용 코드
        UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] GameInstance is null"));
    }

    // 가이드 퀘스트 위젯 생성 직후 한 번 강제 새로고침
    RefreshQuestObjectives();
}

void UQuestWidget::NativeDestruct()
{
    if (QuestComponent)
    {
        QuestComponent->OnQuestUpdated.RemoveAll(this);
        QuestComponent->OnQuestProgressUpdated.RemoveAll(this);
    }

    // 가이드 퀘스트 델리게이트 해제
    if (GuideQuestSubsystem)
    {
        GuideQuestSubsystem->OnGuideQuestUpdated.RemoveAll(this);
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

    //디버깅용 코드
    UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] QuestComponent set and bound"));

    RefreshQuestObjectives();
}

void UQuestWidget::RefreshQuestObjectives()
{
    //디버깅용 코드
    UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] RefreshQuestObjectives called"));

    if (!VB_QuestObjectives)
    {
        //디버깅용 코드
        UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] VB_QuestObjectives is null"));
        return;
    }

    //디버깅용 코드
    UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] QuestObjectiveSlotClass = %s"),
        QuestObjectiveSlotClass ? TEXT("Valid") : TEXT("Null"));

    //디버깅용 코드
    UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] QuestComponent = %s"),
        QuestComponent ? TEXT("Valid") : TEXT("Null"));

    VB_QuestObjectives->ClearChildren();

    bool bHasAnyQuest = false;

    // ============================================
    // Guide Quest
    // ============================================
    if (UGuideQuestSubsystem* GuideSys = GetGameInstance()->GetSubsystem<UGuideQuestSubsystem>())
    {
        //디버깅용 코드
        UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] GuideSys = Valid"));
        UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] HasActiveQuest = %s"),
            GuideSys->HasActiveQuest() ? TEXT("true") : TEXT("false"));

        if (GuideSys->HasActiveQuest())
        {
            //디버깅용 코드
            UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] Guide quest exists"));

            if (QuestObjectiveSlotClass)
            {
                UQuestObjectiveSlot* GuideSlot = CreateWidget<UQuestObjectiveSlot>(this, QuestObjectiveSlotClass);

                if (IsValid(GuideSlot))
                {
                    //디버깅용 코드
                    UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] GuideSlot created successfully"));

                    GuideSlot->SetGuideQuestData(
                        GuideSys->GetActiveQuestTitleText(),
                        GuideSys->BuildObjectiveProgressText(),
                        GuideSys->BuildRewardText()
                    );

                    UVerticalBoxSlot* VBSlot = VB_QuestObjectives->AddChildToVerticalBox(GuideSlot);
                    if (VBSlot)
                    {
                        VBSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
                        VBSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
                    }

                    bHasAnyQuest = true;
                }
                else
                {
                    //디버깅용 코드
                    UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] GuideSlot create failed"));
                }
            }
            else
            {
                //디버깅용 코드
                UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] QuestObjectiveSlotClass is null"));
            }
        }
    }
    else
    {
        //디버깅용 코드
        UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] GuideSys = Null"));
    }

    // ============================================
    // NPC Quest
    // ============================================
    if (QuestComponent)
    {
        TArray<FQuestProgress> ActiveQuests = QuestComponent->GetActiveQuests();

        //디버깅용 코드
        UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] NPC ActiveQuests Num = %d"), ActiveQuests.Num());

        for (const FQuestProgress& Progress : ActiveQuests)
        {
            FQuestDataRow* QuestData = QuestComponent->GetQuestData(Progress.QuestID);
            if (!QuestData) continue;

            // QuestObjectiveSlotClass가 없으면 텍스트 폴백
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
                bHasAnyQuest = true;
                continue;
            }

            // 서브 위젯 생성
            UQuestObjectiveSlot* QuestSlot = CreateWidget<UQuestObjectiveSlot>(this, QuestObjectiveSlotClass);
            if (!QuestSlot) continue;

            QuestSlot->SetObjectiveData(*QuestData, Progress);

            UVerticalBoxSlot* VBSlot = VB_QuestObjectives->AddChildToVerticalBox(QuestSlot);
            if (VBSlot)
            {
                VBSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 6.f));
                VBSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
            }

            bHasAnyQuest = true;
        }
    }

    // 퀘스트 없으면 위젯 숨기기
    if (!bHasAnyQuest)
    {
        //디버깅용 코드
        UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] No Guide/NPC quest. Widget collapsed"));

        SetVisibility(ESlateVisibility::Collapsed);

        if (TXT_NoQuest)
            TXT_NoQuest->SetVisibility(ESlateVisibility::Visible);

        return;
    }

    // 퀘스트 있을 경우
    //디버깅용 코드
    UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] Quest exists. Widget visible"));

    SetVisibility(ESlateVisibility::HitTestInvisible);

    if (TXT_NoQuest)
    {
        TXT_NoQuest->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UQuestWidget::OnQuestUpdated(int32 QuestID, EQuestState NewState)
{
    //디버깅용 코드
    UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] OnQuestUpdated - QuestID: %d"), QuestID);

    RefreshQuestObjectives();
}

void UQuestWidget::OnQuestProgressUpdated(int32 QuestID, int32 CurrentCount)
{
    //디버깅용 코드
    UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] OnQuestProgressUpdated - QuestID: %d, CurrentCount: %d"), QuestID, CurrentCount);

    RefreshQuestObjectives();
}

// 가이드 퀘스트 갱신 시에도 리스트를 다시 그림
void UQuestWidget::OnGuideQuestUpdated()
{
    //디버깅용 코드
    UE_LOG(LogTemp, Warning, TEXT("[QuestWidget] OnGuideQuestUpdated called"));

    RefreshQuestObjectives();
}