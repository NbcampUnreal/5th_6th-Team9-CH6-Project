#include "Subsystem/SBWorldSaveManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Guid.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Character/PlayerAttributeSet.h"

const FString USBWorldSaveManagerSubsystem::IndexSlotName = TEXT("SB_WorldIndex");

// Index Save
USBWorldIndexSaveGame* USBWorldSaveManagerSubsystem::LoadOrCreateIndex()
{
    if (UGameplayStatics::DoesSaveGameExist(IndexSlotName, 0))
    {
        if (auto* Loaded = Cast<USBWorldIndexSaveGame>(UGameplayStatics::LoadGameFromSlot(IndexSlotName, 0)))
        {
            return Loaded;
        }
    }
    return Cast<USBWorldIndexSaveGame>(UGameplayStatics::CreateSaveGameObject(USBWorldIndexSaveGame::StaticClass()));
}

void USBWorldSaveManagerSubsystem::SaveIndex(USBWorldIndexSaveGame* Index)
{
    if (!Index) return;
    UGameplayStatics::SaveGameToSlot(Index, IndexSlotName, 0);
}

FString USBWorldSaveManagerSubsystem::MakeUniqueWorldName(const TArray<FSBWorldSlotMeta>& List, FString BaseName) const
{
    BaseName = BaseName.TrimStartAndEnd();
    if (BaseName.IsEmpty()) BaseName = TEXT("New World");

    TSet<FString> Existing;
    for (const auto& M : List) Existing.Add(M.WorldName);

    if (!Existing.Contains(BaseName)) return BaseName;

    int32 Suffix = 2;
    while (Existing.Contains(FString::Printf(TEXT("%s (%d)"), *BaseName, Suffix)))
    {
        ++Suffix;
    }
    return FString::Printf(TEXT("%s (%d)"), *BaseName, Suffix);
}

TArray<FSBWorldSlotMeta> USBWorldSaveManagerSubsystem::GetWorldList()
{
    USBWorldIndexSaveGame* Index = LoadOrCreateIndex();
    if (!Index) return {};
    return Index->Worlds;
}

bool USBWorldSaveManagerSubsystem::CreateWorld(const FString& InWorldName, FSBWorldSlotMeta& OutCreated)
{
    USBWorldIndexSaveGame* Index = LoadOrCreateIndex();
    if (!Index) return false;

    FSBWorldSlotMeta NewMeta;
    NewMeta.SlotId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
    NewMeta.WorldName = MakeUniqueWorldName(Index->Worlds, InWorldName);
    NewMeta.LastPlayed = FDateTime::Now();
    NewMeta.Day = 1;
    NewMeta.PlayerName = TEXT("Player");
    NewMeta.PlayerLevel = 1;

    Index->Worlds.Add(NewMeta);
    SaveIndex(Index);

    OutCreated = NewMeta;
    return true;
}

bool USBWorldSaveManagerSubsystem::DeleteWorld(const FString& SlotId)
{
    USBWorldIndexSaveGame* Index = LoadOrCreateIndex();
    if (!Index) return false;

    const int32 Removed = Index->Worlds.RemoveAll([&](const FSBWorldSlotMeta& M)
        {
            return M.SlotId == SlotId;
        });

    if (Removed > 0)
    {
        SaveIndex(Index);

        // SlotId로 WorldSave도 저장한다면 같이 삭제 가능
        UGameplayStatics::DeleteGameInSlot(SlotId, 0);

        if (CurrentSlotId == SlotId)
        {
            CurrentSlotId.Reset();
        }
        return true;
    }
    return false;
}

bool USBWorldSaveManagerSubsystem::TouchWorldLastPlayed(const FString& SlotId)
{
    USBWorldIndexSaveGame* Index = LoadOrCreateIndex();
    if (!Index) return false;

    for (FSBWorldSlotMeta& M : Index->Worlds)
    {
        if (M.SlotId == SlotId)
        {
            M.LastPlayed = FDateTime::Now();
            SaveIndex(Index);
            return true;
        }
    }
    return false;
}

bool USBWorldSaveManagerSubsystem::TouchCurrentWorldLastPlayed()
{
    if (CurrentSlotId.IsEmpty()) return false;

    USBWorldIndexSaveGame* Index = LoadOrCreateIndex();
    if (!Index) return false;

    for (FSBWorldSlotMeta& M : Index->Worlds)
    {
        if (M.SlotId == CurrentSlotId)
        {
            M.LastPlayed = FDateTime::Now();
            SaveIndex(Index);
            return true;
        }
    }
    return false;
}

// World Save (SlotId별 실제 월드 저장/로드)
USBWorldSaveGame* USBWorldSaveManagerSubsystem::LoadOrCreateWorldSave(const FString& SlotId)
{
    if (SlotId.IsEmpty()) return nullptr;

    if (UGameplayStatics::DoesSaveGameExist(SlotId, 0))
    {
        if (auto* Loaded = Cast<USBWorldSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotId, 0)))
        {
            if (Loaded->SlotId.IsEmpty())
            {
                Loaded->SlotId = SlotId;
                UGameplayStatics::SaveGameToSlot(Loaded, SlotId, 0);
            }
            return Loaded;
        }
    }

    auto* Created = Cast<USBWorldSaveGame>(UGameplayStatics::CreateSaveGameObject(USBWorldSaveGame::StaticClass()));
    if (!Created) return nullptr;

    Created->SlotId = SlotId;

    Created->bHasPlayerTransform = false;
    Created->PlayerLocation = FVector::ZeroVector;
    Created->PlayerRotation = FRotator::ZeroRotator;

    Created->bHasControlRotation = false;
    Created->SavedControlRotation = FRotator::ZeroRotator;

    Created->Day = 1;

    Created->bHasPlayerAttributes = false;
    Created->SavedHealth = 0.f;
    Created->SavedMaxHealth = 0.f;
    Created->SavedStamina = 0.f;
    Created->SavedMaxStamina = 0.f;
    Created->SavedLevel = 1.f;
    Created->SavedExperience = 0.f;
    Created->SavedAttack = 0.f;
    Created->SavedDefense = 0.f;

    UGameplayStatics::SaveGameToSlot(Created, SlotId, 0);
    return Created;
}

bool USBWorldSaveManagerSubsystem::SaveWorldSave(const FString& SlotId, USBWorldSaveGame* WorldSave)
{
    if (SlotId.IsEmpty() || !WorldSave) return false;
    WorldSave->SlotId = SlotId;
    return UGameplayStatics::SaveGameToSlot(WorldSave, SlotId, 0);
}

// Attributes Save/Load
bool USBWorldSaveManagerSubsystem::FillPlayerAttributesFromPawn(APawn* Pawn, USBWorldSaveGame* Save)
{
    if (!Pawn || !Save) return false;

    IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn);
    UAbilitySystemComponent* ASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;

    if (!ASC)
    {
        Save->bHasPlayerAttributes = false;
        return false;
    }

    Save->bHasPlayerAttributes = true;

    Save->SavedHealth = ASC->GetNumericAttribute(UPlayerAttributeSet::GetHealthAttribute());
    Save->SavedMaxHealth = ASC->GetNumericAttribute(UPlayerAttributeSet::GetMaxHealthAttribute());

    Save->SavedStamina = ASC->GetNumericAttribute(UPlayerAttributeSet::GetStaminaAttribute());
    Save->SavedMaxStamina = ASC->GetNumericAttribute(UPlayerAttributeSet::GetMaxStaminaAttribute());

    Save->SavedLevel = ASC->GetNumericAttribute(UPlayerAttributeSet::GetLevelAttribute());
    Save->SavedExperience = ASC->GetNumericAttribute(UPlayerAttributeSet::GetExperienceAttribute());

    Save->SavedAttack = ASC->GetNumericAttribute(UPlayerAttributeSet::GetAttackAttribute());
    Save->SavedDefense = ASC->GetNumericAttribute(UPlayerAttributeSet::GetDefenseAttribute());

    return true;
}

bool USBWorldSaveManagerSubsystem::ApplyPlayerAttributesToPawn(APawn* Pawn, const USBWorldSaveGame* Save)
{
    if (!Pawn || !Save) return false;
    if (!Save->bHasPlayerAttributes) return true;

    IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn);
    UAbilitySystemComponent* ASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;

    if (!ASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Save] ApplyPlayerAttributesToPawn: ASC is null"));
        return false;
    }

    // Max -> Current 순서 (Clamp 안정)
    ASC->SetNumericAttributeBase(UPlayerAttributeSet::GetMaxHealthAttribute(), Save->SavedMaxHealth);
    ASC->SetNumericAttributeBase(UPlayerAttributeSet::GetHealthAttribute(), Save->SavedHealth);

    ASC->SetNumericAttributeBase(UPlayerAttributeSet::GetMaxStaminaAttribute(), Save->SavedMaxStamina);
    ASC->SetNumericAttributeBase(UPlayerAttributeSet::GetStaminaAttribute(), Save->SavedStamina);

    ASC->SetNumericAttributeBase(UPlayerAttributeSet::GetLevelAttribute(), Save->SavedLevel);
    ASC->SetNumericAttributeBase(UPlayerAttributeSet::GetExperienceAttribute(), Save->SavedExperience);

    ASC->SetNumericAttributeBase(UPlayerAttributeSet::GetAttackAttribute(), Save->SavedAttack);
    ASC->SetNumericAttributeBase(UPlayerAttributeSet::GetDefenseAttribute(), Save->SavedDefense);

    return true;
}

void USBWorldSaveManagerSubsystem::UpdateIndexMetaFromWorldSave(const FString& SlotId, const USBWorldSaveGame* WorldSave)
{
    if (SlotId.IsEmpty() || !WorldSave) return;

    USBWorldIndexSaveGame* Index = LoadOrCreateIndex();
    if (!Index) return;

    for (FSBWorldSlotMeta& M : Index->Worlds)
    {
        if (M.SlotId == SlotId)
        {
            M.LastPlayed = FDateTime::Now();
            M.Day = WorldSave->Day;

            if (WorldSave->bHasPlayerAttributes)
            {
                const int32 Lv = FMath::Max(1, (int32)FMath::FloorToInt(WorldSave->SavedLevel));
                M.PlayerLevel = Lv;
            }

            SaveIndex(Index);
            return;
        }
    }
}

// Public API
bool USBWorldSaveManagerSubsystem::SaveCurrentWorldFromPawn(APawn* Pawn)
{
    if (!Pawn) return false;
    if (CurrentSlotId.IsEmpty()) return false;

    USBWorldSaveGame* Save = LoadOrCreateWorldSave(CurrentSlotId);
    if (!Save) return false;

    // Transform
    Save->bHasPlayerTransform = true;
    Save->PlayerLocation = Pawn->GetActorLocation();
    Save->PlayerRotation = Pawn->GetActorRotation();

    // 카메라 방향
    if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
    {
        Save->bHasControlRotation = true;
        Save->SavedControlRotation = PC->GetControlRotation();
    }
    else
    {
        Save->bHasControlRotation = false;
        Save->SavedControlRotation = FRotator::ZeroRotator;
    }

    // Attributes
    FillPlayerAttributesFromPawn(Pawn, Save);

    const bool bOk = SaveWorldSave(CurrentSlotId, Save);
    if (bOk)
    {
        UpdateIndexMetaFromWorldSave(CurrentSlotId, Save);
    }
    return bOk;
}

bool USBWorldSaveManagerSubsystem::LoadCurrentWorldTransformToPawn(APawn* Pawn)
{
    if (!Pawn) return false;
    if (CurrentSlotId.IsEmpty()) return false;

    USBWorldSaveGame* Save = LoadOrCreateWorldSave(CurrentSlotId);
    if (!Save) return false;

    // Transform
    if (Save->bHasPlayerTransform)
    {
        Pawn->SetActorLocationAndRotation(
            Save->PlayerLocation,
            Save->PlayerRotation,
            false,
            nullptr,
            ETeleportType::TeleportPhysics
        );
    }

    // 카메라 방향
    if (Save->bHasControlRotation)
    {
        if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
        {
            PC->SetControlRotation(Save->SavedControlRotation);
        }
    }

    return true;
}

bool USBWorldSaveManagerSubsystem::LoadCurrentWorldAttributesToPawn(APawn* Pawn)
{
    if (!Pawn) return false;
    if (CurrentSlotId.IsEmpty()) return false;

    USBWorldSaveGame* Save = LoadOrCreateWorldSave(CurrentSlotId);
    if (!Save) return false;

    return ApplyPlayerAttributesToPawn(Pawn, Save);
}

bool USBWorldSaveManagerSubsystem::LoadCurrentWorldToPawn(APawn* Pawn)
{
    if (!Pawn) return false;
    if (CurrentSlotId.IsEmpty()) return false;

    USBWorldSaveGame* Save = LoadOrCreateWorldSave(CurrentSlotId);
    if (!Save) return false;

    bool bOk = true;

    // Transform
    if (Save->bHasPlayerTransform)
    {
        Pawn->SetActorLocationAndRotation(
            Save->PlayerLocation,
            Save->PlayerRotation,
            false,
            nullptr,
            ETeleportType::TeleportPhysics
        );
    }

    // ControlRotation
    if (Save->bHasControlRotation)
    {
        if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
        {
            PC->SetControlRotation(Save->SavedControlRotation);
        }
    }

    // Attributes
    bOk = ApplyPlayerAttributesToPawn(Pawn, Save) && bOk;

    return bOk;
}
