#include "Subsystem/SBWorldSaveManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Guid.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Character/PlayerAttributeSet.h"
#include "Character/PlayerCharacter_SB.h"
#include "Inventory/InventoryComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Build/BuildComponent.h"
#include "EngineUtils.h"
#include "Items/Pickup.h"
#include "Items/ItemBase.h"
#include "Data/ItemData.h"
#include "Data/BuildingData.h"
#include "GuideQuest/GuideQuestSubsystem.h"
#include "Engine/DataTable.h"


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
    Created->bHasInventory = false;
    Created->SavedGold = 1000;

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

bool USBWorldSaveManagerSubsystem::LoadCurrentWorldInventoryToPawn(APawn* Pawn)
{
    if (!Pawn) return false;
    if (CurrentSlotId.IsEmpty()) return false;

    USBWorldSaveGame* Save = LoadOrCreateWorldSave(CurrentSlotId);
    if (!Save) return false;

    return ApplyInventoryToPawn(Pawn, Save);
}

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

bool USBWorldSaveManagerSubsystem::FillInventoryFromPawn(APawn* Pawn, USBWorldSaveGame* Save)
{
    if (!Pawn || !Save) return false;

    APlayerCharacter_SB* PC = Cast<APlayerCharacter_SB>(Pawn);
    if (!PC) { Save->bHasInventory = false; return false; }

    UInventoryComponent* Inv = PC->GetInventory();
    if (!Inv) { Save->bHasInventory = false; return false; }

    Save->bHasInventory = true;
    Inv->BuildSaveData(Save->SavedInventorySlots, Save->SavedHotbarSlots);

    return true;
}

bool USBWorldSaveManagerSubsystem::ApplyInventoryToPawn(APawn* Pawn, const USBWorldSaveGame* Save)
{
    if (!Pawn || !Save) return false;
    if (!Save->bHasInventory) return true;

    APlayerCharacter_SB* PC = Cast<APlayerCharacter_SB>(Pawn);
    if (!PC) return false;

    UInventoryComponent* Inv = PC->GetInventory();
    if (!Inv) return false;

    Inv->ApplySaveData(Save->SavedInventorySlots, Save->SavedHotbarSlots);

    PC->SetGold(Save->SavedGold);

    const int32 HotbarSize = Inv->GetHotbarCapacity();
    const int32 SavedIndex = FMath::Clamp(Save->SavedSelectedHotbarIndex, 0, FMath::Max(0, HotbarSize - 1));
    PC->SelectHotbarIndex(SavedIndex);

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
    FillInventoryFromPawn(Pawn, Save);
    FillPlacedBuildingsFromPawn(Pawn, Save);
    FillDroppedItemsFromPawn(Pawn, Save);

    if (APlayerCharacter_SB* PC = Cast<APlayerCharacter_SB>(Pawn))
    {
        Save->SavedSelectedHotbarIndex = PC->CurrentHotbarIndex;
        Save->SavedGold = PC->GetGold();
    }

    //가이드용 퀘스트 내부 저장 로직
    if (UGameInstance* GI = GetGameInstance()) {
        if (UGuideQuestSubsystem* QuestSys = GI->GetSubsystem<UGuideQuestSubsystem>()) {
            QuestSys->ExportToSaveGame(Save);
        }
    }
    //===============
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

    // ControlRotation (이거 작동 안하는거 같음 수정할 예정)
    if (Save->bHasControlRotation)
    {
        if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
        {
            PC->SetControlRotation(Save->SavedControlRotation);
        }
    }

    // Attributes
    bOk = ApplyPlayerAttributesToPawn(Pawn, Save) && bOk;
    bOk = ApplyInventoryToPawn(Pawn, Save) && bOk;

    //가이드 퀘스트 내부 로드 로직
    if (UGameInstance* GI = GetGameInstance()) {
        if (UGuideQuestSubsystem* QuestSys = GI->GetSubsystem<UGuideQuestSubsystem>()) {
            QuestSys->ImportFromSaveGame(Save);
        }
    }
    //===============
    return bOk;
}

bool USBWorldSaveManagerSubsystem::FillPlacedBuildingsFromPawn(APawn* Pawn, USBWorldSaveGame* Save)
{
    if (!Pawn || !Save) return false;

    Save->SavedBuildings.Reset();

    UWorld* World = Pawn->GetWorld();
    if (!World)
    {
        Save->bHasPlacedBuildings = false;
        return false;
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        if (!Actor->ActorHasTag(TEXT("PlacedBuild")))
        {
            continue;
        }

        FName FoundBuildingID = NAME_None;

        APlayerCharacter_SB* PlayerChar = Cast<APlayerCharacter_SB>(Pawn);
        UBuildComponent* BuildComp = PlayerChar ? PlayerChar->GetBuildComponent() : nullptr;

        if (!BuildComp)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Save] BuildComp is null"));
            Save->bHasPlacedBuildings = false;
            return false;
        }

        for (const FName& Tag : Actor->Tags)
        {
            if (Tag == TEXT("PlacedBuild"))
            {
                continue;
            }

            FBuildingDataRow DummyRow;
            if (BuildComp->GetBuildingData(Tag, DummyRow))
            {
                FoundBuildingID = Tag;
                break;
            }
        }

        if (FoundBuildingID.IsNone())
        {
            UE_LOG(LogTemp, Warning, TEXT("[Save] PlacedBuild has no BuildingID tag: %s"), *GetNameSafe(Actor));
            continue;
        }

        FSBPlacedBuildingSaveData Data;
        Data.BuildingID = FoundBuildingID;
        Data.Transform = Actor->GetActorTransform();

        Save->SavedBuildings.Add(Data);
    }

    Save->bHasPlacedBuildings = Save->SavedBuildings.Num() > 0;

    UE_LOG(LogTemp, Warning, TEXT("[Save] Buildings Saved = %d"), Save->SavedBuildings.Num());
    return true;
}

bool USBWorldSaveManagerSubsystem::ApplyPlacedBuildingsToPawn(APawn* Pawn, const USBWorldSaveGame* Save)
{
    if (!Pawn || !Save) return false;
    if (!Save->bHasPlacedBuildings) return true;

    APlayerCharacter_SB* PC = Cast<APlayerCharacter_SB>(Pawn);
    if (!PC) return false;

    UBuildComponent* BuildComp = PC->GetBuildComponent();
    if (!BuildComp) return false;

    UWorld* World = Pawn->GetWorld();
    if (!World) return false;

    // 혹시 기존 배치 건축물이 있으면 제거
    TArray<AActor*> ToDestroy;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor && Actor->ActorHasTag(TEXT("PlacedBuild")))
        {
            ToDestroy.Add(Actor);
        }
    }

    for (AActor* Actor : ToDestroy)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }

    // 저장된 건축물 복원
    for (const FSBPlacedBuildingSaveData& Data : Save->SavedBuildings)
    {
        if (Data.BuildingID.IsNone())
        {
            continue;
        }

        FBuildingDataRow Row;
        if (!BuildComp->GetBuildingData(Data.BuildingID, Row))
        {
            UE_LOG(LogTemp, Warning, TEXT("[Load] Building row not found: %s"), *Data.BuildingID.ToString());
            continue;
        }

        if (!Row.BuildActorClass)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Load] BuildActorClass is null: %s"), *Data.BuildingID.ToString());
            continue;
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = PC;
        SpawnParams.Instigator = PC;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AActor* Spawned = World->SpawnActor<AActor>(Row.BuildActorClass, Data.Transform, SpawnParams);
        if (!Spawned)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Load] Failed to spawn building: %s"), *Data.BuildingID.ToString());
            continue;
        }

        Spawned->Tags.AddUnique(TEXT("PlacedBuild"));
        Spawned->Tags.AddUnique(Data.BuildingID);
    }

    UE_LOG(LogTemp, Warning, TEXT("[Load] Buildings Loaded = %d"), Save->SavedBuildings.Num());
    return true;
}

bool USBWorldSaveManagerSubsystem::LoadCurrentWorldBuildingsToPawn(APawn* Pawn)
{
    if (!Pawn) return false;
    if (CurrentSlotId.IsEmpty()) return false;

    USBWorldSaveGame* Save = LoadOrCreateWorldSave(CurrentSlotId);
    if (!Save) return false;

    return ApplyPlacedBuildingsToPawn(Pawn, Save);
}

bool USBWorldSaveManagerSubsystem::LoadCurrentWorldDroppedItemsToPawn(APawn* Pawn)
{
    if (!Pawn) return false;
    if (CurrentSlotId.IsEmpty()) return false;

    USBWorldSaveGame* Save = LoadOrCreateWorldSave(CurrentSlotId);
    if (!Save) return false;

    return ApplyDroppedItemsToPawn(Pawn, Save);
}

bool USBWorldSaveManagerSubsystem::FillDroppedItemsFromPawn(APawn* Pawn, USBWorldSaveGame* Save)
{
    if (!Pawn || !Save) return false;

    Save->SavedDroppedItems.Reset();

    UWorld* World = Pawn->GetWorld();
    if (!World)
    {
        Save->bHasDroppedItems = false;
        return false;
    }

    for (TActorIterator<APickup> It(World); It; ++It)
    {
        APickup* Pickup = *It;
        if (!Pickup) continue;

        if (!Pickup->ActorHasTag(TEXT("SavedWorldDrop")))
        {
            continue;
        }

        UItemBase* Item = Pickup->GetItemData();
        if (!Item)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Save] Pickup has no ItemData: %s"), *GetNameSafe(Pickup));
            continue;
        }

        if (Item->ID.IsNone() || Item->Quantity <= 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Save] Invalid dropped item data: %s"), *GetNameSafe(Pickup));
            continue;
        }

        FSBWorldDroppedItemSaveData Data;
        Data.ItemID = Item->ID;
        Data.Quantity = Item->Quantity;
        Data.Transform = Pickup->GetActorTransform();

        Save->SavedDroppedItems.Add(Data);
    }

    Save->bHasDroppedItems = Save->SavedDroppedItems.Num() > 0;

    UE_LOG(LogTemp, Warning, TEXT("[Save] DroppedItems Saved = %d"), Save->SavedDroppedItems.Num());
    return true;
}

UItemBase* USBWorldSaveManagerSubsystem::CreateWorldDropItemFromID(
    UDataTable* ItemDataTable,
    FName ItemID,
    int32 Quantity,
    UObject* Outer
) const
{
    if (!ItemDataTable || ItemID.IsNone() || Quantity <= 0 || !Outer)
    {
        return nullptr;
    }

    const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(ItemID, TEXT("CreateWorldDropItemFromID"));
    if (!ItemData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Load] Item row not found: %s"), *ItemID.ToString());
        return nullptr;
    }

    UItemBase* NewItem = NewObject<UItemBase>(Outer);
    if (!NewItem)
    {
        return nullptr;
    }

    NewItem->ID = ItemData->ID;
    NewItem->ItemType = ItemData->ItemType;
    NewItem->ItemQuality = ItemData->ItemQuality;
    NewItem->ItemStatistics = ItemData->ItemStatistics;
    NewItem->TextData = ItemData->TextData;
    NewItem->NumericData = ItemData->NumericData;
    NewItem->AssetData = ItemData->AssetData;
    NewItem->PickupActorClass = ItemData->PickupActorClass;
    NewItem->EquipWeaponClass = ItemData->EquipWeaponClass;
    NewItem->ConsumableEffectClass = ItemData->ConsumableEffectClass;
    NewItem->ConsumableSetByCallerTag = ItemData->ConsumableSetByCallerTag;

    NewItem->ResetItemFlags();
    NewItem->OwningInventory = nullptr;
    NewItem->Quantity = FMath::Clamp(
        Quantity,
        1,
        ItemData->NumericData.bIsStackable ? ItemData->NumericData.MaxStackSize : 1
    );

    return NewItem;
}

bool USBWorldSaveManagerSubsystem::ApplyDroppedItemsToPawn(APawn* Pawn, const USBWorldSaveGame* Save)
{
    if (!Pawn || !Save) return false;
    if (!Save->bHasDroppedItems) return true;

    APlayerCharacter_SB* PlayerChar = Cast<APlayerCharacter_SB>(Pawn);
    if (!PlayerChar) return false;

    TSubclassOf<APickup> WorldPickupClass = PlayerChar->GetPickupClass();
    if (!WorldPickupClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Load] ApplyDroppedItemsToPawn: PickupClass is null"));
        return false;
    }

    UInventoryComponent* Inv = PlayerChar->GetInventory();
    if (!Inv || !Inv->ItemDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Load] ApplyDroppedItemsToPawn: ItemDataTable is null"));
        return false;
    }

    for (const FSBWorldDroppedItemSaveData& Data : Save->SavedDroppedItems)
    {
        if (Data.ItemID.IsNone() || Data.Quantity <= 0) continue;

        UItemBase* DropItem = CreateWorldDropItemFromID(
            Inv->ItemDataTable,
            Data.ItemID,
            Data.Quantity,
            Pawn
        );

        if (!DropItem) continue;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        APickup* Pickup = Pawn->GetWorld()->SpawnActor<APickup>(
            WorldPickupClass,
            Data.Transform,
            Params
        );

        if (!Pickup) continue;

        Pickup->InitializeDrop(DropItem, Data.Quantity);
        Pickup->Tags.AddUnique(TEXT("SavedWorldDrop"));
    }

    UE_LOG(LogTemp, Warning, TEXT("[Load] DroppedItems Loaded = %d"), Save->SavedDroppedItems.Num());
    return true;
}