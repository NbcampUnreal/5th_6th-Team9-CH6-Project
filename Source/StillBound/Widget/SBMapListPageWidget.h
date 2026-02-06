#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SBMapListPageWidget.generated.h"

class UListView;
class UButton;
class UCanvasPanel;
class UTextBlock;
class USBWorldSlotData;
class USBConfirmDialogWidget;
class USBNameInputDialogWidget;

UCLASS()
class STILLBOUND_API USBMapListPageWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Ref")
    UUserWidget* TitleRootRef = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Popup")
    TSubclassOf<USBConfirmDialogWidget> ConfirmDialogClass;

    UPROPERTY(EditDefaultsOnly, Category = "Popup")
    TSubclassOf<USBNameInputDialogWidget> NameInputDialogClass;

    UFUNCTION(BlueprintCallable) void BP_OnWorldSelected(UObject* Item);
    UFUNCTION(BlueprintCallable) void BP_OnWorldDoubleClicked(UObject* Item);

    UPROPERTY(EditDefaultsOnly, Category = "Navigation")
    FName TitleLevelName = TEXT("TitleLevel");

    UPROPERTY(EditDefaultsOnly, Category = "Travel")
    FName GameplayLevelName = NAME_None;

protected:
    UPROPERTY(meta = (BindWidget)) UListView* LV_Worlds;

    UPROPERTY(meta = (BindWidget)) UButton* BTN_Create;
    UPROPERTY(meta = (BindWidget)) UButton* BTN_Delete;
    UPROPERTY(meta = (BindWidget)) UButton* BTN_Join;
    UPROPERTY(meta = (BindWidget)) UButton* BTN_Back;

    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* TXT_Empty;
    UPROPERTY(meta = (BindWidgetOptional)) UCanvasPanel* CP_PopupLayer;

    UPROPERTY() USBWorldSlotData* Selected = nullptr;

    virtual void NativeConstruct() override;

    void RefreshList_FromSave();

    UFUNCTION() void OnCreateClicked();
    UFUNCTION() void OnDeleteClicked();
    UFUNCTION() void OnJoinClicked();
    UFUNCTION() void OnBackClicked();

    UFUNCTION() void HandleDeleteConfirmed();
    UFUNCTION() void HandleCreateNameConfirmed(FString WorldName);
};