//GatherableObject.cpp

#include "GatherableObject.h"
#include "Character/PlayerCharacter_SB.h"
#include "Inventory/InventoryComponent.h"
#include "Items/Pickup.h"
#include "Items/ItemBase.h"
#include "TimerManager.h"   
#include "Engine/World.h"
#include "Engine/DataTable.h"

AGatherableObject::AGatherableObject()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
}

void AGatherableObject::BeginPlay()
{
    Super::BeginPlay();
    CurrentGatherCount = MaxGatherCount;
}

void AGatherableObject::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 레벨 전환/종료 시 타이머 안전하게 정리
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(RespawnTimerHandle);
    }
    Super::EndPlay(EndPlayReason);
}

//======== IInteractionInterface 구현부

void AGatherableObject::BeginFocus_Implementation()
{
    // 하이라이트 ON
    MeshComponent->SetRenderCustomDepth(true);
    BP_OnBeginFocus();
}

void AGatherableObject::EndFocus_Implementation()
{
    // 하이라이트 OFF
    MeshComponent->SetRenderCustomDepth(false);
    BP_OnEndFocus();
}

void AGatherableObject::BeginInteract_Implementation()
{
    if (!bIsActive) { return; }

    //월드에서 플레이어 찾기
    APlayerCharacter_SB* Player = nullptr;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (APlayerController* PC = It->Get())
        {
            Player = Cast<APlayerCharacter_SB>(PC->GetPawn());
            if (Player) { break; }
        }
    }

    if (!Player) { return; }

    if (!CanGather(Player))
    {
        InteractableData.InteractionDuration = 0.f;
        return;
    }

    int32 ToolTier = GetCharacterToolTier(Player);
     // InteractableData의 Duration을 도구 티어에 따라 갱신
     InteractableData.InteractionDuration = CalculateGatherTime(ToolTier);

     Player->NotifyGatherStart(InteractableData.InteractionDuration);
}

void AGatherableObject::EndInteract_Implementation()
{
    //채집 종료 알림(취소에도 호출)
    APlayerCharacter_SB* Player = nullptr;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (APlayerController* PC = It->Get())
        {
            Player = Cast<APlayerCharacter_SB>(PC->GetPawn());
            if (Player) { break; }
        }
    }
    if (Player) Player->NotifyGatherEnd();
}

void AGatherableObject::Interact_Implementation(APlayerCharacter_SB* PlayerCharacter)
{
    // InteractionDuration 후 이 함수가 호출됨 = 채집 완료

    if (!PlayerCharacter) { return; }
    if (!bIsActive) { return; }
    if (!CanGather(PlayerCharacter)) { return; }

    //월드에 pickup 드랍
    
    SpawnDropItems(PlayerCharacter);
    // 아이템 지급
    //GiveItemsToCharacter(PlayerCharacter);

    // 채집 횟수 감소
    CurrentGatherCount--;

    BP_OnGatherComplete();

    PlayerCharacter->NotifyGatherEnd();

    // 인터랙션 위젯 갱신
    PlayerCharacter->UpdateInteractionWidget();

    // 채집 횟수 소진 시 소멸
    if (CurrentGatherCount <= 0)
    {
        DeactivateObject();
    }
}

FInteractableData AGatherableObject::GetInteractableData_Implementation()
{
    //인터렉션에서 추가한 채집 타입 지정
    InteractableData.InteractableType = EInteractableType::Gatherable;
    // 비활성 상태
    if (!bIsActive)
    { 
        InteractableData.InteractionDuration = 0.f;
        InteractableData.Name = FText::FromString(TEXT("Regenerating..."));
        InteractableData.Action = FText::FromString(TEXT(""));
       
        return InteractableData;
    }

    // 이름/액션 텍스트 설정
    if (GatherType == EGatherType::Wood)
    {
        InteractableData.Name = FText::FromString(
            FString::Printf(TEXT("Wood (remaining foraging %d)"), CurrentGatherCount));
    }
    else
    {
        InteractableData.Name = FText::FromString(
            FString::Printf(TEXT("Rock (remaining foraging %d)"), CurrentGatherCount));
    }

    InteractableData.Action = FText::FromString(TEXT("gathering"));
    // Duration은 BeginInteract_Implementation에서 갱신됨, 여기엔 기본값 세팅
    InteractableData.InteractionDuration = BaseGatherTime;

    return InteractableData;
}

// ===============채집 가능 여부 체크
bool AGatherableObject::CanGather(APlayerCharacter_SB* Character) const
{
    if (!Character) return false;

    int32 ToolTier = GetCharacterToolTier(Character);

    if (GatherType == EGatherType::Wood)
    {
        // 나무: 맨손(0)은 1티어 나무만 가능
        // 도구 티어 >= ObjectTier - 1
        return ToolTier >= (ObjectTier - 1);
    }
    else // Rock
    {
        // 돌: 맨손 불가, 도구 티어 >= ObjectTier
        if (ToolTier == 0) return false;
        return ToolTier >= ObjectTier;
    }
}

// =============도구 티어 가져오기
int32 AGatherableObject::GetCharacterToolTier(APlayerCharacter_SB* Character) const
{
    if (!Character) { return 0; }

    UInventoryComponent* Inventory = Character->GetInventory();
    if (!Inventory) { return 0; }

    // 현재 핫바에서 선택된 아이템 가져오기,  GetItemInContainer는 기존 InventoryComponent에 있는 함수 사용
    UItemBase* EquippedItem = Inventory->GetItemInContainer(
        ESlotContainer::Hotbar,
        Character->CurrentHotbarIndex
    );

    if (!EquippedItem) { return 0; }
    if (EquippedItem->ItemType != EItemType::Tool) { return 0; }

    // ToolStatDataTable에서 티어 조회
    if (!ToolStatDataTable) { return 0; }

    FToolStatRow* ToolRow = ToolStatDataTable->FindRow<FToolStatRow>(
        EquippedItem->ID, TEXT("GetToolTier")
    );
    if (!ToolRow) { return 0; }

    return ToolRow->Tier;
}

// ============채집 시간 계산
float AGatherableObject::CalculateGatherTime(int32 ToolTier) const
{
    // 티어 1당 0.5초 감소, 최소 0.5초
    float TimeReduction = ToolTier * 0.5f;
    return FMath::Max(BaseGatherTime - TimeReduction, 0.5f);
}

// =========아이템 드랍
void AGatherableObject::SpawnDropItems(APlayerCharacter_SB* Character)
{
    if (!Character || !ItemDataTable || !PickupClass) { return; }

    //UInventoryComponent* Inventory = Character->GetInventory();
    //if (!Inventory) return;

    UWorld* World = GetWorld();
    if (!World) { return; }

    for (const FGatherDropItem& Drop : DropItems)
    {
        int32 Count = FMath::RandRange(Drop.MinCount, Drop.MaxCount);
        if (Count <= 0) continue;

        FItemDataRow* ItemRow = ItemDataTable->FindRow<FItemDataRow>(
            Drop.ItemID, TEXT("GatherDrop")
        );
        if (!ItemRow) continue;

        //UItemBase 생성
        UItemBase* NewItem = NewObject<UItemBase>(this);
        NewItem->ID                              = ItemRow->ID;
        NewItem->ItemType                  = ItemRow->ItemType;
        NewItem->ItemQuality             = ItemRow->ItemQuality;
        NewItem->ItemStatistics        = ItemRow->ItemStatistics;
        NewItem->TextData                 = ItemRow->TextData;
        NewItem->NumericData         = ItemRow->NumericData;
        NewItem->AssetData              = ItemRow->AssetData;
        NewItem->PickupActorClass = ItemRow->PickupActorClass;
        NewItem->Quantity = Count;

        //기존 InventoryComponent의 AddItem 함수명으로 수정
       // Inventory->AddItem(NewItem);

        //오브젝트 주변 랜덤 위치에 겹치지 않게 약간 퍼트려서 스폰
        FVector SpawnOffset = FVector(
            FMath::RandRange(-50.f, 50.f),
            FMath::RandRange(-50.f, 50.f),
            50.f
        );

        FVector SpawnLocation = GetActorLocation() + SpawnOffset;
        FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        APickup* SpawnedPickup = World->SpawnActor<APickup>(PickupClass, SpawnTransform, SpawnParams);
        if (SpawnedPickup)
        {
            SpawnedPickup->InitializeDrop(NewItem, Count);
        }
    }
}

// ===========오브젝트 소멸
void AGatherableObject::DeactivateObject()
{
    bIsActive = false;

    MeshComponent->SetVisibility(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComponent->SetRenderCustomDepth(false);

    BP_OnDepleted();

    UWorld* World = GetWorld();
    if (World && RespawnTime > 0.f)
    {
        FTimerDelegate RespawnDelegate;
        RespawnDelegate.BindUObject(this, &AGatherableObject::OnRespawn);
        World->GetTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, RespawnTime, false);
    }
}

// ===========리스폰(오브젝트 리스폰 안 시킬거면 주석처리)

void AGatherableObject::OnRespawn()
{
    ActivateObject();
}

void AGatherableObject::ActivateObject()
{
    bIsActive = true;
    CurrentGatherCount = MaxGatherCount;

    MeshComponent->SetVisibility(true);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    BP_OnRespawned();
}