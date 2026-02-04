
#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter_SB.h"
#include "PlayerCharacter_SB.generated.h"

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

UCLASS()
class STILLBOUND_API APlayerCharacter_SB : public ABaseCharacter_SB
{
	GENERATED_BODY()
	
public:
	APlayerCharacter_SB();

	UTextureRenderTarget2D* GetMiniMapTarget() const { return MiniMapTarget; }

	FORCEINLINE bool IsInteracting() const { return GetWorldTimerManager().IsTimerActive(TimerHandle_Interaction); };

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TScriptInterface<IInteractionInterface> TargetInteractable;

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
