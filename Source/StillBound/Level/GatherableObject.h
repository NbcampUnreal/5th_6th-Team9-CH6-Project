#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractionInterface.h"   // ���� �������̽�
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

//������ ��� �Ӽ� ���� ����
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
    virtual float GetInteractionDistance_Implementation() override;
public:
    // ====================FŰ ������ �� IInteractionInterface ����, ���� �ý��� ����

    // �÷��̾ �ٶ� ��
    virtual void BeginFocus_Implementation() override;
    virtual void EndFocus_Implementation() override;

    // FŰ ������ �� ,BeginInteract���� ȣ��
    virtual void BeginInteract_Implementation() override;
    virtual void EndInteract_Implementation() override;

    // InteractionDuration �� ���� ä�� �Ϸ� ,Interact���� ȣ��
    virtual void Interact_Implementation(APlayerCharacter_SB* PlayerCharacter) override;

    // ���ͷ��� ������ ǥ���� ������ ��ȯ
    virtual FInteractableData GetInteractableData_Implementation() override;

    // ================ä�� ����
    UFUNCTION()
    void OnRespawn();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Gather")
    bool IsActive() const { return bIsActive; }

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gather")
    class USphereComponent* InteractionCollision;

protected:
    // ============������ ������
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    EGatherType GatherType = EGatherType::Wood;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    int32 ObjectTier = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    int32 MaxGatherCount = 5;

    // �⺻ ä�� �ð� �Ǽ�/1Ƽ�� ���� , InteractionDuration���� �Ѱ��� ��
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    float BaseGatherTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    float RespawnTime = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    TArray<FGatherDropItem> DropItems;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    UDataTable* ItemDataTable;

    // ToolStatTable ���� Ƽ�� ��ȸ��, ���� �Ⱦ�
    //UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    //UDataTable* ToolStatDataTable;

    //pickup ���� Ŭ����
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gather|Setup")
    TSubclassOf<APickup> PickupClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    // =======��Ÿ�� ����

    //InteractableData�� ����� ���� ����
    UPROPERTY(EditAnywhere, Category = "Interaction")
    FInteractableData InteractableData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gather|State")
    int32 CurrentGatherCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gather|State")
    bool bIsActive = true;

    FTimerHandle RespawnTimerHandle;

    //======== ���� �Լ�

    // ĳ���� ���� ������ Ƽ�� ��ȯ ,0=�Ǽ�
    int32 GetCharacterToolTier(APlayerCharacter_SB* Character) const;

    // ���� Ƽ� ���� ä�� �ð� ���
    float CalculateGatherTime(int32 ToolTier) const;

    // ä�� ���� ���� ,Ƽ�� üũ
    bool CanGather(APlayerCharacter_SB* Character) const;

    // ������ �κ��丮�� ����
    //void GiveItemsToCharacter(APlayerCharacter_SB* Character);

    //��� ������
    void SpawnDropItems(APlayerCharacter_SB* Character);

    void DeactivateObject();
    void ActivateObject();

    //============ �������Ʈ �̺�Ʈ (���־�/�����)
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

    //�������� ����̿�
    FTimerHandle FallTimerHandle;

    //������ �� �������� ó��
    UFUNCTION()
    void OnFallComplete();

    //BP���� �������� ���� ���۰� FallDuration���� �޽� ����.
    UFUNCTION(BlueprintImplementableEvent, Category = "Gather")
    void BP_OnStartFalling(float FallDuration);
};