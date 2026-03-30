#include "Widget/SBMapListPageWidget.h"

#include "Components/ListView.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Data/SBWorldSlotData.h"
#include "Widget/SBConfirmDialogWidget.h"
#include "Widget/SBNameInputDialogWidget.h"
#include "Subsystem/SBWorldSaveManagerSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameInstance/SBGameInstance.h"

void USBMapListPageWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (BTN_Create) BTN_Create->OnClicked.AddDynamic(this, &ThisClass::OnCreateClicked);
    if (BTN_Delete) BTN_Delete->OnClicked.AddDynamic(this, &ThisClass::OnDeleteClicked);
    if (BTN_Join)   BTN_Join->OnClicked.AddDynamic(this, &ThisClass::OnJoinClicked);
    if (BTN_Back)   BTN_Back->OnClicked.AddDynamic(this, &ThisClass::OnBackClicked);

    Selected = nullptr;
    if (BTN_Delete) BTN_Delete->SetIsEnabled(false);
    if (BTN_Join)   BTN_Join->SetIsEnabled(false);

    RefreshList_FromSave();
}

void USBMapListPageWidget::RefreshList_FromSave()
{
    if (!LV_Worlds) return;

    LV_Worlds->ClearListItems();
    LV_Worlds->ClearSelection();

    Selected = nullptr;
    if (BTN_Delete) BTN_Delete->SetIsEnabled(false);
    if (BTN_Join)   BTN_Join->SetIsEnabled(false);

    auto* Sub = GetGameInstance() ? GetGameInstance()->GetSubsystem<USBWorldSaveManagerSubsystem>() : nullptr;
    if (!Sub)
    {
        if (TXT_Empty) TXT_Empty->SetVisibility(ESlateVisibility::Visible);
        return;
    }

    const TArray<FSBWorldSlotMeta> List = Sub->GetWorldList();

    for (const FSBWorldSlotMeta& M : List)
    {
        LV_Worlds->AddItem(
            USBWorldSlotData::Make(
                this,
                M.SlotId,
                M.WorldName,
                M.LastPlayed,
                M.Day,
                M.PlayerName,
                M.PlayerLevel
            )
        );
    }

    const bool bHasAny = (LV_Worlds->GetNumItems() > 0);
    if (TXT_Empty) TXT_Empty->SetVisibility(bHasAny ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}

void USBMapListPageWidget::BP_OnWorldSelected(UObject* Item)
{
    Selected = Cast<USBWorldSlotData>(Item);

    const bool bValid = (Selected != nullptr);
    if (BTN_Delete) BTN_Delete->SetIsEnabled(bValid);
    if (BTN_Join)   BTN_Join->SetIsEnabled(bValid);
}

void USBMapListPageWidget::BP_OnWorldDoubleClicked(UObject* Item)
{
    Selected = Cast<USBWorldSlotData>(Item);
    OnJoinClicked();
}

void USBMapListPageWidget::OnCreateClicked()
{
    if (!NameInputDialogClass) return;

    auto* Popup = CreateWidget<USBNameInputDialogWidget>(GetOwningPlayer(), NameInputDialogClass);
    if (!Popup) return;

    Popup->SetTitle(FText::FromString(TEXT("Create World")));
    Popup->OnOk.AddDynamic(this, &ThisClass::HandleCreateNameConfirmed);

    if (CP_PopupLayer)
    {
        if (auto* CanvasSlot = CP_PopupLayer->AddChildToCanvas(Popup))
        {
            CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
            CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
            CanvasSlot->SetAutoSize(true);
            CanvasSlot->SetZOrder(200);
        }
    }
    else
    {
        Popup->AddToViewport(100);
    }
}

void USBMapListPageWidget::HandleCreateNameConfirmed(FString WorldName)
{
    auto* Sub = GetGameInstance() ? GetGameInstance()->GetSubsystem<USBWorldSaveManagerSubsystem>() : nullptr;
    if (!Sub) return;

    FSBWorldSlotMeta Created;
    if (!Sub->CreateWorld(WorldName, Created))
        return;

    RefreshList_FromSave();
}

void USBMapListPageWidget::OnDeleteClicked()
{
    if (!Selected || !ConfirmDialogClass) return;

    auto* Popup = CreateWidget<USBConfirmDialogWidget>(GetOwningPlayer(), ConfirmDialogClass);
    if (!Popup) return;

    //Popup->SetMessage(FText::FromString(TEXT("Delete?")));
    Popup->OnYes.AddDynamic(this, &ThisClass::HandleDeleteConfirmed);

    if (CP_PopupLayer)
    {
        if (auto* CanvasSlot = CP_PopupLayer->AddChildToCanvas(Popup))
        {
            CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
            CanvasSlot->SetOffsets(FMargin(0.f));
            CanvasSlot->SetZOrder(300);
        }
    }
    else
    {
        Popup->AddToViewport(100);
    }
}

void USBMapListPageWidget::HandleDeleteConfirmed()
{
    if (!Selected) return;

    auto* Sub = GetGameInstance() ? GetGameInstance()->GetSubsystem<USBWorldSaveManagerSubsystem>() : nullptr;
    if (!Sub) return;

    Sub->DeleteWorld(Selected->SlotId);

    RefreshList_FromSave();
}

void USBMapListPageWidget::OnJoinClicked()
{
    if (!Selected) return;

    auto* Sub = GetGameInstance() ? GetGameInstance()->GetSubsystem<USBWorldSaveManagerSubsystem>() : nullptr;
    if (!Sub) return;

    Sub->SetCurrentSlotId(Selected->SlotId);

    if (GameplayLevelName.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("GameplayLevelName is None. Set it in WBP_MapListPage defaults."));
        return;
    }

    if (USBGameInstance* GI = GetGameInstance<USBGameInstance>())
    {
        GI->RequestLoadingScreenOnce();
    }

    UGameplayStatics::OpenLevel(this, GameplayLevelName);
}
void USBMapListPageWidget::OnBackClicked()
{
    //UGameplayStatics::OpenLevel(GetWorld(), TitleLevelName);
    //RemoveFromParent();
}
