#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractionInterface.h"   // 기존 인터페이스
#include "Data/ItemData.h"
#include "GatherableObject.generated.h"

UENUM(BlueprintType)
enum class EGatherType : uint8
{
    Wood  UMETA(DisplayName = "Wood"),
    Rock  UMETA(DisplayName = "Rock")
};

USTRUCT(BlueprintType)
struct FGatherDropItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gather")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gather")
    int32 MinCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gather")
    int32 MaxCount = 1;
};

//아이템 드롭 속성 전방 선언
class APickup;

UCLASS()
class STILLBOUND_API AGatherableObject : public AActor, public IInteractionInterface
{
    GENERATED_BODY()

public:
    AGatherableObject();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // ====================F키 눌렀을 때 IInteractionInterface 구현, 기존 시스템 연결

    // 플레이어가 바라볼 때
    virtual void BeginFocus_Implementation() override;
    virtual void EndFocus_Implementation() override;

    // F키 눌렀을 때 ,BeginInteract에서 호출
    virtual void BeginInteract_Implementation() override;
    virtual void EndInteract_Implementation() override;

    // InteractionDuration 후 실제 채집 완료 ,Interact에서 호출
    virtual void Interact_Implementation(APlayerCharacter_SB* PlayerCharacter) override;

    // 인터랙션 위젯에 표시할 데이터 반환
    virtual FInteractableData GetInteractableData_Implementation() override;

    // ================채집 관련
    UFUNCTION()
    void OnRespawn();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Gather")
    bool IsActive() const { return bIsActive; }

protected:
    // ============에디터 설정값
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    EGatherType GatherType = EGatherType::Wood;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    int32 ObjectTier = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    int32 MaxGatherCount = 5;

    // 기본 채집 시간 맨손/1티어 기준 , InteractionDuration으로 넘겨줄 값
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    float BaseGatherTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    float RespawnTime = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    TArray<FGatherDropItem> DropItems;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    UDataTable* ItemDataTable;

    // ToolStatTable 도구 티어 조회용
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    UDataTable* ToolStatDataTable;

    //pickup 액터 클래스
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    TSubclassOf<APickup> PickupClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    // =======런타임 상태

    //InteractableData를 멤버로 직접 선언
    UPROPERTY(EditAnywhere, Category = "Interaction")
    FInteractableData InteractableData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gather|State")
    int32 CurrentGatherCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gather|State")
    bool bIsActive = true;

    FTimerHandle RespawnTimerHandle;

    //======== 내부 함수

    // 캐릭터 장착 도구의 티어 반환 ,0=맨손
    int32 GetCharacterToolTier(APlayerCharacter_SB* Character) const;

    // 도구 티어에 따른 채집 시간 계산
    float CalculateGatherTime(int32 ToolTier) const;

    // 채집 가능 여부 ,티어 체크
    bool CanGather(APlayerCharacter_SB* Character) const;

    // 아이템 인벤토리에 지급
    //void GiveItemsToCharacter(APlayerCharacter_SB* Character);

    //드롭 아이템
    void SpawnDropItems(APlayerCharacter_SB* Character);

    void DeactivateObject();
    void ActivateObject();

    //============ 블루프린트 이벤트 (비주얼/사운드용)
    UFUNCTION(BlueprintImplementableEvent, Category = "Gather")
    void BP_OnGatherComplete();

    UFUNCTION(BlueprintImplementableEvent, Category = "Gather")
    void BP_OnDepleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Gather")
    void BP_OnRespawned();

    UFUNCTION(BlueprintImplementableEvent, Category = "Gather")
    void BP_OnBeginFocus();

    UFUNCTION(BlueprintImplementableEvent, Category = "Gather")
    void BP_OnEndFocus();
};