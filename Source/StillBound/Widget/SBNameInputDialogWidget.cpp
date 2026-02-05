#include "Widget/SBNameInputDialogWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void USBNameInputDialogWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (BTN_OK) BTN_OK->OnClicked.AddDynamic(this, &ThisClass::HandleOk);
    if (BTN_Cancel) BTN_Cancel->OnClicked.AddDynamic(this, &ThisClass::HandleCancel);
}

void USBNameInputDialogWidget::SetTitle(const FText& InTitle)
{
    if (TXT_Title) TXT_Title->SetText(InTitle);
}

void USBNameInputDialogWidget::HandleOk()
{
    FString Name;
    if (ETB_WorldName) Name = ETB_WorldName->GetText().ToString().TrimStartAndEnd();

    OnOk.Broadcast(Name);
    RemoveFromParent();
}

void USBNameInputDialogWidget::HandleCancel()
{
    OnCancel.Broadcast();
    RemoveFromParent();
}
