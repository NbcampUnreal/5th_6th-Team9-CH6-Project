//GatherableObject.cpp

#include "GatherableObject.h"
#include "Character/PlayerCharacter_SB.h"
#include "Inventory/InventoryComponent.h"
#include "Items/Pickup.h"
#include "Items/ItemBase.h"
#include "TimerManager.h"   
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"

AGatherableObject::AGatherableObject()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    // 오브젝트 크기에 상관없이 표면 근처에서 상호작용 가능
    //InteractionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCollision"));
    //InteractionCollision->SetupAttachment(RootComponent);
    //InteractionCollision->SetSphereRadius(150.f);        // BP에서 오브젝트 크기에 맞게 조절
    //InteractionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    //InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    //InteractionCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 캐릭터만 감지

    ////Block 응답 추가. PerformInteractionCheck의 LineTrace(ECC_Visibility)가 이 SphereComponent에 먼저 닿음
    //InteractionCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    //InteractionCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
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
        World->GetTimerManager().ClearTimer(FallTimerHandle);
        World->GetTimerManager().ClearTimer(HotbarCheckTimerHandle);
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
        InteractableData.InteractionDuration = 9999.f;
        return;
    }

    int32 ToolTier = GetCharacterToolTier(Player);
     // InteractableData의 Duration을 도구 티어에 따라 갱신
     InteractableData.InteractionDuration = CalculateGatherTime(ToolTier);

     //채집 시작 시 현재 핫바 상태 캐싱
     CachedHotbarIndex = Player->CurrentHotbarIndex;
     UInventoryComponent* Inventory = Player->GetInventory();
     if (Inventory)
     {
         UItemBase* EquippedItem = Inventory->GetItemInContainer(
             ESlotContainer::Hotbar, CachedHotbarIndex);
         CachedToolID = EquippedItem ? EquippedItem->ID : NAME_None;
     }

     // 핫바 변경 감지 타이머 시작 (0.1초마다 체크)
     UWorld* World = GetWorld();
     if (World)
     {
         World->GetTimerManager().SetTimer(
             HotbarCheckTimerHandle,
             this,
             &AGatherableObject::CheckHotbarChanged,
             0.1f,
             true 
         );
     }

     Player->NotifyGatherStart(InteractableData.InteractionDuration);
}

void AGatherableObject::EndInteract_Implementation()
{
    // 핫바 체크 타이머 정리
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(HotbarCheckTimerHandle);
    }

    // 캐시 초기화
    CachedHotbarIndex = -1;
    CachedToolID = NAME_None;

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

    //플레이어를 찾아서 현재 도구 티어 반영
    APlayerCharacter_SB* Player = nullptr;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (APlayerController* PC = It->Get())
        {
            Player = Cast<APlayerCharacter_SB>(PC->GetPawn());
            if (Player) { break; }
        }
    }

    if (Player)
    {
        //채집 불가 상태면 Duration 0으로 표시
        if (!CanGather(Player))
        {
            InteractableData.InteractionDuration = 0.f;
            InteractableData.Action = FText::FromString(TEXT("Cannot gather"));
        }
        else
        {
            int32 ToolTier = GetCharacterToolTier(Player);
            InteractableData.InteractionDuration = CalculateGatherTime(ToolTier);
            InteractableData.Action = FText::FromString(TEXT("gathering"));
        }
    }
    else
    {
        InteractableData.InteractionDuration = BaseGatherTime;
        InteractableData.Action = FText::FromString(TEXT("gathering"));
    }

    return InteractableData;
}

// ===============채집 가능 여부 체크
bool AGatherableObject::CanGather(APlayerCharacter_SB* Character) const
{
    if (!Character) return false;

    UInventoryComponent* Inventory = Character->GetInventory();
    if (!Inventory) return false;

    UItemBase* EquippedItem = Inventory->GetItemInContainer(
        ESlotContainer::Hotbar,
        Character->CurrentHotbarIndex
    );

    //도구를 들고 있는 경우 종류 체크
    if (EquippedItem && EquippedItem->ItemType == EItemType::Tool)
    {
        EToolKind EquippedKind = EquippedItem->ItemStatistics.ToolKind;

        // 도끼로 돌 채집 불가
        if (GatherType == EGatherType::Rock && EquippedKind == EToolKind::Axe)
        {
            return false;
        }

        // 곡괭이로 나무 채집 불가
        if (GatherType == EGatherType::Wood && EquippedKind == EToolKind::Pickaxe)
        {
            return false;
        }
    }

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

//핫바 변경 확인
void AGatherableObject::CheckHotbarChanged()
{
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

    bool bShouldCancel = false;

    //핫바 인덱스 변경 감지
    if (Player->CurrentHotbarIndex != CachedHotbarIndex)
    {
        bShouldCancel = true;
    }
    else
    {
        //같은 슬롯이라도 아이템이 바뀐 경우 감지
        UInventoryComponent* Inventory = Player->GetInventory();
        if (Inventory)
        {
            UItemBase* CurrentItem = Inventory->GetItemInContainer(
                ESlotContainer::Hotbar, CachedHotbarIndex);
            FName CurrentID = CurrentItem ? CurrentItem->ID : NAME_None;

            if (CurrentID != CachedToolID)
            {
                bShouldCancel = true;
            }
        }
    }

    if (bShouldCancel)
    {
        //타이머 먼저 정리후 EndInteract호출
        UWorld* World = GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(HotbarCheckTimerHandle);
        }

        // 캐시 초기화
        CachedHotbarIndex = -1;
        CachedToolID = NAME_None;
        //캐릭터의 EndInteract 호출해서 채집 취소
        Player->EndInteract();
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

    //// ToolStatDataTable에서 티어 조회
    //if (!ToolStatDataTable) { return 0; }

    //FToolStatRow* ToolRow = ToolStatDataTable->FindRow<FToolStatRow>(
    //    EquippedItem->ID, TEXT("GetToolTier")
    //);
    //if (!ToolRow) { return 0; }

    //return ToolRow->Tier;

   // 오브젝트 타입에 맞는 도구 종류 확인, 나무는 도끼, 돌은 곡괭이만 채집 속도 감소 적용
    EToolKind RequiredKind = (GatherType == EGatherType::Wood)
        ? EToolKind::Axe
        : EToolKind::Pickaxe;

    // 도구 종류 불일치 시 0 반환, 맨손 취급
    if (EquippedItem->ItemStatistics.ToolKind != RequiredKind)
    {
        return 0;
    }
    //ItemStatistics.ObjectTier 직접 사용으로 수정
    return (int32)EquippedItem->ItemStatistics.ObjectTier;
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

    //캐릭터 기준 드랍 스폰
    FVector CharacterLocation = Character->GetActorLocation();

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

        FVector WorldOffset = Character->GetActorRotation().RotateVector(SpawnOffset);

        // 라인트레이스로 지면 높이 감지
        FVector TraceStart = CharacterLocation + WorldOffset + FVector(0.f, 0.f, 100.f);
        FVector TraceEnd = CharacterLocation + WorldOffset - FVector(0.f, 0.f, 300.f);

        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Character);
        QueryParams.AddIgnoredActor(this);

        FVector SpawnLocation;
        if (World->LineTraceSingleByChannel(
            HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
        {
            SpawnLocation = HitResult.ImpactPoint + FVector(0.f, 0.f, 10.f); // 지면 위 10cm
        }
        else
        {
            // 지면 감지 실패 시 캐릭터 발 위치로 fallback
            SpawnLocation = CharacterLocation + WorldOffset;
            SpawnLocation.Z = CharacterLocation.Z;
        }

        FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        APickup* SpawnedPickup = World->SpawnActor<APickup>(
            PickupClass, SpawnTransform, SpawnParams);
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

    //충돌만 제거 쓰러지는동안 상호작용 불가능한 목적용
    //MeshComponent->SetVisibility(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComponent->SetRenderCustomDepth(false);

    //쓰러지는 시간, BP의 타임라인 길이와 맞춰야됨.
    const float FallDuration = 1.5f;

    //BP에 쓰러지기 신호 시작
    BP_OnStartFalling(FallDuration);

    //FallDuration 후 메시 숨기고 리스폰 타이머 시작
    UWorld* World = GetWorld();
    if (World)
    {
        FTimerDelegate FallDelegate;
        FallDelegate.BindUObject(this, &AGatherableObject::OnFallComplete);
        World->GetTimerManager().SetTimer(FallTimerHandle, FallDelegate, FallDuration, false);
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

    //회전값 초기화
    MeshComponent->SetRelativeRotation(FRotator::ZeroRotator);

    MeshComponent->SetVisibility(true);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    BP_OnRespawned();
}

//나무가 쓰러진 뒤 리스폰 타이머 시작
void AGatherableObject::OnFallComplete()
{
    MeshComponent->SetVisibility(false);

    BP_OnDepleted();//기존 이벤트, 파티클 사운드

    //리스폰 타이머 시작
    UWorld* World = GetWorld();
    if (World && RespawnTime > 0.f)
    {
        FTimerDelegate RespawnDelegate;
        RespawnDelegate.BindUObject(this, &AGatherableObject::OnRespawn);
        World->GetTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, RespawnTime, false);
    }
}

float AGatherableObject::GetInteractionDistance_Implementation()
{
    /*if (!InteractionCollision) { return 225.f; }*/

    // SphereRadius + 여유 50cm 반환
    /*return InteractionCollision->GetScaledSphereRadius() + 150.f;*/
    return 500.f;
}