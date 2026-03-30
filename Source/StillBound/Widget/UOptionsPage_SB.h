#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Framework/Commands/InputChord.h"
#include "UOptionsPage_SB.generated.h"

class UInputKeySelector;
class UEnhancedInputLocalPlayerSubsystem;

UCLASS()
class STILLBOUND_API UOptionsPage_SB : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    UInputKeySelector* BuildKeySelector;

    UPROPERTY(meta = (BindWidget))
    UInputKeySelector* InteractKeySelector;

    UPROPERTY(meta = (BindWidget))
    UInputKeySelector* InventoryKeySelector;

    UPROPERTY(meta = (BindWidget))
    UInputKeySelector* DodgeKeySelector;

    UPROPERTY(meta = (BindWidget))
    UInputKeySelector* WorldMapKeySelector;

    UFUNCTION()
    void OnBuildKeySelected(FInputChord SelectedKey);

    UFUNCTION()
    void OnInteractKeySelected(FInputChord SelectedKey);

    UFUNCTION()
    void OnInventoryKeySelected(FInputChord SelectedKey);

    UFUNCTION()
    void OnDodgeKeySelected(FInputChord SelectedKey);

    UFUNCTION()
    void OnWorldMapKeySelected(FInputChord SelectedKey);

public:
    UFUNCTION(BlueprintCallable)
    void ForceRefreshKeyBindingsUI();

    UFUNCTION(BlueprintCallable)
    bool ApplyPendingKeyBindings();

    UFUNCTION(BlueprintCallable)
    void RevertPendingKeyBindings();

    UFUNCTION(BlueprintCallable)
    bool ResetAllKeyBindingsToDefault();

private:
    void RefreshKeyBindingsUIFromPending();
    void CaptureCurrentBindings();
    bool FindMappedKey(const FName MappingName, FKey& OutKey) const;
    UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;

    void HandleKeySelected(const FName MappingName, UInputKeySelector* Selector, const FInputChord& SelectedKey);
    bool HasDuplicatePendingKey(const FName MappingName, const FKey& NewKey, FName* OutConflictRow = nullptr) const;
    bool ApplyOneMapping(const FName MappingName, const FKey& NewKey);

private:
    TMap<FName, FKey> PendingBindings;
    TMap<FName, FKey> AppliedBindingsOnOpen;

    bool bIgnoreSelectorEvents = false;
};