
#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter_SB.h"
#include "Interface/InteractionInterface.h"
#include "Data/InventoryTypes.h"
#include "PlayerCharacter_SB.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int32, NewGold);

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

	// ===== 기본 무기 설정 (BP에서 설정) =====
	// 게임 시작 시 생성할 무기 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SB|Weapon")
	TSubclassOf<AWeaponBase> StartingWeaponClass;

	// 캐릭터 스켈레탈 메시(손)에 설정된 소켓 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SB|Weapon")
	FName StartingWeaponSocketName = TEXT("WeaponSocket");
	
	// 런타임에 생성된 무기 인스턴스
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SB|Weapon")
	TObjectPtr<AWeaponBase> EquippedWeapon;

	// PC(플레이어 컨트롤러) 등 외부에서 현재 무기를 참조할 수 있는 Getter
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

	// 시작 무기 장착 로직 실행 함수
	void EquipStartingWeapon(); 

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void UnequipWeapon(bool bDestroyWeaponActor = true);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool EquipWeaponFromItem(UItemBase* Item);

	// ? PC�� ���� ���⸦ ������ �� �ְ� Getter ����
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

	int32 CurrentHotbarIndex = 0;

	// 골드 시스템
	UFUNCTION(BlueprintCallable, Category = "Player|Gold")
	int32 GetGold() const { return CurrentGold; }

	UFUNCTION(BlueprintCallable, Category = "Player|Gold")
	bool ModifyGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Player|Gold")
	void SetGold(int32 NewAmount);

	UFUNCTION(BlueprintCallable, Category = "Player|Gold")
	bool HasEnoughGold(int32 Amount) const { return CurrentGold >= Amount; }


private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UAnimMontage> DeathMontage;

	bool bIsDead = false;

	// 골드 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Gold",
		meta = (ClampMin = "0", AllowPrivateAccess="true"))
	int32 CurrentGold = 1000;  // 시작 골드

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Gold", meta = (AllowPrivateAccess = "true"))
	int32 MaxGold = 999999;

	// 골드 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Player|Gold")
	FOnGoldChanged OnGoldChanged;

	UPROPERTY()
	TObjectPtr<UItemBase> SelectedConsumable = nullptr;

};
