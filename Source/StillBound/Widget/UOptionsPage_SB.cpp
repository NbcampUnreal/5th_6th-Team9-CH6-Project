#include "Widget/UOptionsPage_SB.h"

#include "Components/InputKeySelector.h"
#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "GameplayTagContainer.h"
#include "InputCoreTypes.h"

void UOptionsPage_SB::NativeConstruct()
{
    Super::NativeConstruct();

    if (BuildKeySelector)
    {
        BuildKeySelector->OnKeySelected.AddDynamic(this, &ThisClass::OnBuildKeySelected);
    }

    if (InteractKeySelector)
    {
        InteractKeySelector->OnKeySelected.AddDynamic(this, &ThisClass::OnInteractKeySelected);
    }

    if (InventoryKeySelector)
    {
        InventoryKeySelector->OnKeySelected.AddDynamic(this, &ThisClass::OnInventoryKeySelected);
    }

    if (DodgeKeySelector)
    {
        DodgeKeySelector->OnKeySelected.AddDynamic(this, &ThisClass::OnDodgeKeySelected);
    }

    if (WorldMapKeySelector)
    {
        WorldMapKeySelector->OnKeySelected.AddDynamic(this, &ThisClass::OnWorldMapKeySelected);
    }
}

UEnhancedInputLocalPlayerSubsystem* UOptionsPage_SB::GetEnhancedInputSubsystem() const
{
    if (!GetOwningPlayer())
    {
        return nullptr;
    }

    ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
    if (!LocalPlayer)
    {
        return nullptr;
    }

    return ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
}

bool UOptionsPage_SB::FindMappedKey(const FName MappingName, FKey& OutKey) const
{
    const UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
    if (!Subsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KeyUI] Subsystem is null"));
        return false;
    }

    const UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings();
    if (!UserSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KeyUI] UserSettings is null"));
        return false;
    }

    const UEnhancedPlayerMappableKeyProfile* Profile = UserSettings->GetCurrentKeyProfile();
    if (!Profile)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KeyUI] CurrentKeyProfile is null"));
        return false;
    }

    TArray<FKey> Keys;
    const int32 NumKeys = Profile->GetMappedKeysInRow(MappingName, Keys);

    UE_LOG(LogTemp, Warning, TEXT("[KeyUI] Profile row %s -> %d keys"),
        *MappingName.ToString(), NumKeys);

    for (const FKey& Key : Keys)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KeyUI] Profile Name=%s Key=%s"),
            *MappingName.ToString(),
            *Key.ToString());

        if (Key.IsValid() && Key != EKeys::Invalid)
        {
            OutKey = Key;
            return true;
        }
    }

    return false;
}

void UOptionsPage_SB::CaptureCurrentBindings()
{
    PendingBindings.Empty();
    AppliedBindingsOnOpen.Empty();

    auto CaptureOne = [this](const FName MappingName)
        {
            FKey FoundKey;
            if (FindMappedKey(MappingName, FoundKey))
            {
                PendingBindings.Add(MappingName, FoundKey);
                AppliedBindingsOnOpen.Add(MappingName, FoundKey);
            }
        };

    CaptureOne(TEXT("BuildMode"));
    CaptureOne(TEXT("Interact"));
    CaptureOne(TEXT("Inventory"));
    CaptureOne(TEXT("Dodge"));
    CaptureOne(TEXT("Map"));
}

void UOptionsPage_SB::RefreshKeyBindingsUIFromPending()
{
    bIgnoreSelectorEvents = true;

    auto SetSelector = [this](UInputKeySelector* Selector, const FName MappingName)
        {
            if (!Selector)
            {
                return;
            }

            if (const FKey* FoundKey = PendingBindings.Find(MappingName))
            {
                Selector->SetSelectedKey(FInputChord(*FoundKey));
            }
            else
            {
                Selector->SetSelectedKey(FInputChord());
            }
        };

    SetSelector(BuildKeySelector, TEXT("BuildMode"));
    SetSelector(InteractKeySelector, TEXT("Interact"));
    SetSelector(InventoryKeySelector, TEXT("Inventory"));
    SetSelector(DodgeKeySelector, TEXT("Dodge"));
    SetSelector(WorldMapKeySelector, TEXT("Map"));

    bIgnoreSelectorEvents = false;
}

void UOptionsPage_SB::ForceRefreshKeyBindingsUI()
{
    CaptureCurrentBindings();
    RefreshKeyBindingsUIFromPending();
}

// 중복키 검사는 나중에 구현.
// 지금은 일단 항상 false 반환해서 Apply / Back 구조부터 안정화.
bool UOptionsPage_SB::HasDuplicatePendingKey(const FName MappingName, const FKey& NewKey, FName* OutConflictRow) const
{
    return false;
}

void UOptionsPage_SB::HandleKeySelected(const FName MappingName, UInputKeySelector* Selector, const FInputChord& SelectedKey)
{
    if (bIgnoreSelectorEvents)
    {
        return;
    }

    if (!Selector)
    {
        return;
    }

    const FKey NewKey = SelectedKey.Key;
    if (!NewKey.IsValid() || NewKey == EKeys::Invalid)
    {
        RefreshKeyBindingsUIFromPending();
        return;
    }

    PendingBindings.FindOrAdd(MappingName) = NewKey;

    UE_LOG(LogTemp, Warning, TEXT("[KeyUI] Pending %s -> %s"),
        *MappingName.ToString(),
        *NewKey.ToString());
}

bool UOptionsPage_SB::ApplyOneMapping(const FName MappingName, const FKey& NewKey)
{
    UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
    if (!Subsystem)
    {
        return false;
    }

    UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings();
    if (!UserSettings)
    {
        return false;
    }

    FMapPlayerKeyArgs Args;
    Args.MappingName = MappingName;
    Args.NewKey = NewKey;
    Args.Slot = EPlayerMappableKeySlot::First;

    FGameplayTagContainer FailureReason;
    UserSettings->MapPlayerKey(Args, FailureReason);

    if (!FailureReason.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[KeyApply] Failed: %s -> %s"),
            *MappingName.ToString(),
            *NewKey.ToString());
        return false;
    }

    return true;
}

bool UOptionsPage_SB::ApplyPendingKeyBindings()
{
    UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
    if (!Subsystem)
    {
        return false;
    }

    UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings();
    if (!UserSettings)
    {
        return false;
    }

    for (const TPair<FName, FKey>& Pair : PendingBindings)
    {
        if (!ApplyOneMapping(Pair.Key, Pair.Value))
        {
            return false;
        }
    }

    UserSettings->ApplySettings();
    UserSettings->AsyncSaveSettings();

    AppliedBindingsOnOpen = PendingBindings;
    RefreshKeyBindingsUIFromPending();

    UE_LOG(LogTemp, Warning, TEXT("[KeyApply] Pending bindings applied"));
    return true;
}

void UOptionsPage_SB::RevertPendingKeyBindings()
{
    PendingBindings = AppliedBindingsOnOpen;
    RefreshKeyBindingsUIFromPending();

    UE_LOG(LogTemp, Warning, TEXT("[KeyApply] Pending bindings reverted"));
}

void UOptionsPage_SB::OnBuildKeySelected(FInputChord SelectedKey)
{
    HandleKeySelected(TEXT("BuildMode"), BuildKeySelector, SelectedKey);
}

void UOptionsPage_SB::OnInteractKeySelected(FInputChord SelectedKey)
{
    HandleKeySelected(TEXT("Interact"), InteractKeySelector, SelectedKey);
}

void UOptionsPage_SB::OnInventoryKeySelected(FInputChord SelectedKey)
{
    HandleKeySelected(TEXT("Inventory"), InventoryKeySelector, SelectedKey);
}

void UOptionsPage_SB::OnDodgeKeySelected(FInputChord SelectedKey)
{
    HandleKeySelected(TEXT("Dodge"), DodgeKeySelector, SelectedKey);
}

void UOptionsPage_SB::OnWorldMapKeySelected(FInputChord SelectedKey)
{
    HandleKeySelected(TEXT("Map"), WorldMapKeySelector, SelectedKey);
}

bool UOptionsPage_SB::ResetAllKeyBindingsToDefault()
{
    UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
    if (!Subsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KeyReset] Subsystem is null"));
        return false;
    }

    UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings();
    if (!UserSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("[KeyReset] UserSettings is null"));
        return false;
    }

    static const FName Rows[] =
    {
        TEXT("BuildMode"),
        TEXT("Interact"),
        TEXT("Inventory"),
        TEXT("Dodge"),
        TEXT("Map")
    };

    for (const FName& RowName : Rows)
    {
        FMapPlayerKeyArgs Args;
        Args.MappingName = RowName;
        Args.Slot = EPlayerMappableKeySlot::First;

        FGameplayTagContainer FailureReason;
        UserSettings->ResetAllPlayerKeysInRow(Args, FailureReason);

        if (!FailureReason.IsEmpty())
        {
            UE_LOG(LogTemp, Warning, TEXT("[KeyReset] Failed to reset row: %s"), *RowName.ToString());
            return false;
        }
    }

    UserSettings->ApplySettings();
    UserSettings->AsyncSaveSettings();

    ForceRefreshKeyBindingsUI();

    UE_LOG(LogTemp, Warning, TEXT("[KeyReset] All key bindings reset to default"));
    return true;
}