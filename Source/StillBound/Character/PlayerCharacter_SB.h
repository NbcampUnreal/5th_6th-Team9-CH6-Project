
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
class USB_UIManager;
class APickup;

UCLASS()
class STILLBOUND_API APlayerCharacter_SB : public ABaseCharacter_SB, public IInteractionInterface
{
	GENERATED_BODY()
	
public:
	APlayerCharacter_SB();

	FORCEINLINE bool IsInteracting() const { return GetWorldTimerManager().IsTimerActive(TimerHandle_Interaction) || InteractionData.bIsInteracting;};

	FORCEINLINE UInventoryComponent* GetInventory() const { return PlayerInventory; };

	void UpdateInteractionWidget() const;

	void DropItemFromSlot(ESlotContainer FromContainer, int32 FromIndex, int32 QuantityToDrop);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UInventoryComponent> PlayerInventory;

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TScriptInterface<IInteractionInterface> TargetInteractable;

	UPROPERTY(EditDefaultsOnly, Category="Drop")
	TSubclassOf<APickup> PickupClass;

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

	bool ApplyConsumablePotionGE(UItemBase* Item);

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
};
