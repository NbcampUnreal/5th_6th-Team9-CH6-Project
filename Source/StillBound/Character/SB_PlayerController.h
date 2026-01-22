
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "SB_PlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class STILLBOUND_API ASB_PlayerController : public APlayerController
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
	void Interact();

	void Attack();
	void Skill();

	void SelectHotbar1();
	void SelectHotbar2();
	void SelectHotbar3();
	void SelectHotbar4();
};
