
#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter_SB.h"
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
};

class UCameraComponent;
class USpringArmComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UInventoryComponent;
class IInteractionInterface;
class AWeaponBase;
class USB_UIManager;

UCLASS()
class STILLBOUND_API APlayerCharacter_SB : public ABaseCharacter_SB
{
	GENERATED_BODY()
	
public:
	APlayerCharacter_SB();

	UTextureRenderTarget2D* GetMiniMapTarget() const { return MiniMapTarget; }

	FORCEINLINE bool IsInteracting() const { return GetWorldTimerManager().IsTimerActive(TimerHandle_Interaction); };

	FORCEINLINE UInventoryComponent* GetInventory() const { return PlayerInventory; };

	void UpdateInteractionWidget() const;

	void DropItem(UItemBase* ItemToDrop, const int32 QuantityToDrop);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UInventoryComponent> PlayerInventory;

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TScriptInterface<IInteractionInterface> TargetInteractable;

public:
	float InteractionCheckFrequency;

	float InteractionCheckDistance;

	FTimerHandle TimerHandle_Interaction;

	FInteractionData InteractionData;

	void PerformInteractionCheck();
	void FoundInteractable(AActor* NewInteractable);
	void NoInteractableFound();
	void BeginInteract();
	void EndInteract();
	void Interact();

	UFUNCTION(BlueprintCallable)
	void Die();

	// ===== 시작 무기 세팅 (BP에서 지정) =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SB|Weapon")
	TSubclassOf<AWeaponBase> StartingWeaponClass;

	// 캐릭터 스켈레탈메시(손)에 만든 소켓명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SB|Weapon")
	FName StartingWeaponSocketName = TEXT("WeaponSocket");

	// 런타임 장착된 무기
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SB|Weapon")
	TObjectPtr<AWeaponBase> EquippedWeapon;

	// ? PC가 현재 무기를 가져갈 수 있게 Getter 제공
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	AWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

	void EquipStartingWeapon(); // 추가



private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	//Minimap Camera
	UPROPERTY(VisibleAnywhere, Category = "MiniMap")
	TObjectPtr<USpringArmComponent> MiniMapArm;

	UPROPERTY(VisibleAnywhere, Category = "MiniMap")
	TObjectPtr<USceneCaptureComponent2D> MiniMapCapture;

	UPROPERTY(EditDefaultsOnly, Category = "MiniMap")
	TObjectPtr<UTextureRenderTarget2D> MiniMapTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UAnimMontage> DeathMontage;

	bool bIsDead = false;
};
