
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/USB_UIManager.h"
#include "GameFramework/PlayerController.h"
#include "PlayerController_SB.generated.h"


class UInputMappingContext;
class UInputAction;
class USB_UIManager;
struct FInputActionValue;

UCLASS()
class STILLBOUND_API APlayerController_SB : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void SetupInputComponent() override;

private:
	/// =========================
	/// Input - Mapping Contexts
	/// =========================
	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Movement")
	TArray<TObjectPtr<UInputMappingContext>> InputMappingContexts;

	/// =========================
	/// Input - Movement
	/// =========================
	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Movement")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Movement")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Movement")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Movement")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Movement")
	TObjectPtr<UInputAction> EvasionAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Movement")
	TObjectPtr<UInputAction> InteractAction;

	/// =========================
	/// Input - Abilities
	/// =========================
	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Abilities")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Abilities")
	TObjectPtr<UInputAction> SkillAction;

	/// =========================
	/// Input - Hotbar
	/// =========================
	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Hotbar")
	TObjectPtr<UInputAction> Hotbar1Action;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Hotbar")
	TObjectPtr<UInputAction> Hotbar2Action;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Hotbar")
	TObjectPtr<UInputAction> Hotbar3Action;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Hotbar")
	TObjectPtr<UInputAction> Hotbar4Action;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|ToggleMenu")
	TObjectPtr<UInputAction> ToggleMenuAction;


private:
	/// =========================
	/// Input Handlers
	/// =========================
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void Jump();
	void StopJumping();

	void ToggleCrouch();
	void Evasion();
	void BeginInteract();
	void EndInteract();

	bool ActivateAbility(const FGameplayTag& AbilityTag) const;

	void Attack();
	void Skill();

	void SelectHotbar1();
	void SelectHotbar2();
	void SelectHotbar3();
	void SelectHotbar4();

	void ToggleMenu();

public:

	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(EditDefaultsOnly, Category = "SB|UI")
	TSubclassOf<class USB_UIManager> UIManagerClass;

	UPROPERTY()
	TObjectPtr<class USB_UIManager> UIManager;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Stamina|Cost")
	float EvasionStaminaCost = 25.f;

	UFUNCTION()
	void OnHealthChanged(float OldValue, float NewValue);

	UFUNCTION()
	void OnStaminaChanged(float OldValue, float NewValue);


};
