
#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter_SB.h"
#include "Interface/InteractionInterface.h"
#include "Data/InventoryTypes.h"
#include "PlayerCharacter_SB.generated.h"

class UItemBase;

USTRUCT()
struct FInteractionData
{
	GENERATED_BODY()

	FInteractionData() :
		CurrentInteractable(nullptr),
		LastInteractionCheckTime(0.f)
	{

	};

	UPROPERTY()
	AActor* CurrentInteractable;

	UPROPERTY()
	float LastInteractionCheckTime;

	UPROPERTY()
	bool bIsInteracting = false;
};

class UCameraComponent;
class USpringArmComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UInventoryComponent;
class IInteractionInterface;
class AWeaponBase;
class APickup;
class UBuildComponent;

UCLASS()
class STILLBOUND_API APlayerCharacter_SB : public ABaseCharacter_SB, public IInteractionInterface
{
	GENERATED_BODY()
	
public:
	APlayerCharacter_SB();

	FORCEINLINE bool IsInteracting() const { return GetWorldTimerManager().IsTimerActive(TimerHandle_Interaction) || InteractionData.bIsInteracting;};

	FORCEINLINE UInventoryComponent* GetInventory() const { return PlayerInventory; };

	FORCEINLINE UBuildComponent* GetBuildComponent() const { return BuildComponent; };

	void UpdateInteractionWidget() const;

	void DropItemFromSlot(ESlotContainer FromContainer, int32 FromIndex, int32 QuantityToDrop);
	
	void DestroyActorComponent(UActorComponent* ComponentToDestroy);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UInventoryComponent> PlayerInventory;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UBuildComponent> BuildComponent;

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TScriptInterface<IInteractionInterface> TargetInteractable;

	UPROPERTY(EditDefaultsOnly, Category="Drop")
	TSubclassOf<APickup> PickupClass;
	
	//사망 애니메이션 관련
	UFUNCTION(BlueprintImplementableEvent, Category = "Death", meta = (DisplayName = "On Death Animation Finished"))
	void K2_OnDeathAnimationFinished();

public:
	float InteractionCheckFrequency;

	float InteractionCheckDistance;

	FTimerHandle TimerHandle_Interaction;

	FInteractionData InteractionData;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	FInteractableData InteractableData;

	virtual FInteractableData GetInteractableData_Implementation() override;

	void PerformInteractionCheck();
	void FoundInteractable(AActor* NewInteractable);
	void NoInteractableFound();
	void BeginInteract();
	void EndInteract();
	void Interact();
	void SelectHotbarIndex(int32 NewIndex);
	void HandleHotbarSelectionChanged();
	void UseSelectedHotbarItem();

	UFUNCTION(BlueprintCallable)
	void OpenCraftingUI(FName InStationTag, UDataTable* InRecipeTable);

	UFUNCTION(BlueprintCallable)
	void Die();

	// ===== ���� ���� ���� (BP���� ����) =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SB|Weapon")
	TSubclassOf<AWeaponBase> StartingWeaponClass;

	// ĳ���� ���̷�Ż�޽�(��)�� ���� ���ϸ�
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SB|Weapon")
	FName StartingWeaponSocketName = TEXT("WeaponSocket");

	// ��Ÿ�� ������ ����
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SB|Weapon")
	TObjectPtr<AWeaponBase> EquippedWeapon;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void UnequipWeapon(bool bDestroyWeaponActor = true);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool EquipWeaponFromItem(UItemBase* Item);

	// ? PC�� ���� ���⸦ ������ �� �ְ� Getter ����
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

	

	int32 CurrentHotbarIndex = 0;

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UAnimMontage> DeathMontage;

	bool bIsDead = false;

	UPROPERTY()
	TObjectPtr<UItemBase> SelectedConsumable = nullptr;

	//============채집 기능 추가
public:
	//채집 이동 감지
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Gather")
	bool bIsGathering = false;

	// 채집 시작 위치 저장
	FVector GatherStartLocation;

	//채집 시작/종료 알림, GatherableObject에서 호출
	void NotifyGatherStart(float Duration);
	void NotifyGatherEnd();
};
