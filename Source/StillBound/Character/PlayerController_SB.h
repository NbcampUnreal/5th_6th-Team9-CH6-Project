
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/USB_UIManager.h"
#include "GameFramework/PlayerController.h"
#include "PlayerController_SB.generated.h"


class UInputMappingContext;
class UInputAction;
class USB_UIManager;
class AMapWorldManager;
struct FInputActionValue;

UENUM(BlueprintType)
enum class EOverlayInputState : uint8
{
	Gameplay     UMETA(DisplayName = "Gameplay"),
	Inventory    UMETA(DisplayName = "Inventory"),
	Crafting     UMETA(DisplayName = "Crafting"),
	BuildMenu    UMETA(DisplayName = "BuildMenu"),
	BuildPreview UMETA(DisplayName = "BuildPreview"),
	FullMap      UMETA(DisplayName = "FullMap"),
	PauseMenu    UMETA(DisplayName = "PauseMenu")
};

UCLASS()
class STILLBOUND_API APlayerController_SB : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void SetupInputComponent() override;

private:
	UPROPERTY(VisibleAnywhere, Category="SB|UI")
	EOverlayInputState OverlayState = EOverlayInputState::Gameplay;

	/// =========================
	/// Input - Mapping Contexts
	/// =========================
	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Contexts")
	TObjectPtr<UInputMappingContext> IMC_System;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Contexts")
	TObjectPtr<UInputMappingContext> IMC_Movement;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Contexts")
	TObjectPtr<UInputMappingContext> IMC_Abilities;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Contexts")
	TObjectPtr<UInputMappingContext> IMC_Hotbar;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Contexts")
	TObjectPtr<UInputMappingContext> IMC_BuildPreviewMode;

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
	TObjectPtr<UInputAction> EmoteAction;

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
	TObjectPtr<UInputAction> MouseWheelAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Hotbar")
	TObjectPtr<UInputAction> HotbarSelectAction_1;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Hotbar")
	TObjectPtr<UInputAction> HotbarSelectAction_2;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Hotbar")
	TObjectPtr<UInputAction> HotbarSelectAction_3;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Hotbar")
	TObjectPtr<UInputAction> HotbarSelectAction_4;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Hotbar")
	TObjectPtr<UInputAction> HotbarSelectAction_5;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Hotbar")
	TObjectPtr<UInputAction> HotbarSelectAction_6;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Hotbar")
	TObjectPtr<UInputAction> HotbarSelectAction_7;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Hotbar")
	TObjectPtr<UInputAction> HotbarSelectAction_8;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Hotbar")
	TObjectPtr<UInputAction> HotbarSelectAction_9
		;
	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Hotbar")
	TObjectPtr<UInputAction> UseHotbarAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|ToggleMenu")
	TObjectPtr<UInputAction> ToggleMenuAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|UI")
	TObjectPtr<UInputAction> FullMapAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Build")
	TObjectPtr<UInputAction> ToggleBuildAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Build")
	TObjectPtr<UInputAction> PlaceBuildAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Build")
	TObjectPtr<UInputAction> CancelBuildAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Build")
	TObjectPtr<UInputAction> RaiseBuildAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Build")
	TObjectPtr<UInputAction> LowerBuildAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|System")
	TObjectPtr<UInputAction> ESCAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Build")
	TObjectPtr<UInputAction> BuildRotateAction;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Input|Build")
	TObjectPtr<UInputAction> BuildDestroyAction;
	/// =========================
	/// UI Map
	/// =========================
	UPROPERTY()
	TObjectPtr<AMapWorldManager> MapWorldManager;
public:
	virtual void Tick(float DeltaTime) override;


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
	void Emote();
	void CancelEmoteAbility();
	void BeginInteract();
	void EndInteract();

	bool ActivateAbility(const FGameplayTag& AbilityTag) const;

	bool IsGameplayInputBlocked() const;


	UFUNCTION(BlueprintCallable, Category = "SB|Abilities")
	bool ActivateAbilityAttack(const FGameplayTag& InputTag) const;

	void Attack();

	void Skill();

	void OnEscapePressed();

	void ToggleMenu();

	void OnMouseWheel(const FInputActionValue& Value);
	void OnHotbar1();
	void OnHotbar2();
	void OnHotbar3();
	void OnHotbar4();
	void OnHotbar5();
	void OnHotbar6();
	void OnHotbar7();
	void OnHotbar8();
	void OnHotbar9();
	void OnUseHotbar(const FInputActionValue& Value);

	void ToggleBuild();
	void OnBuildPlace();
	void OnBuildCancel();

	void RaiseBuildHeight();
	void LowerBuildHeight();

	void HandleBuildRotate(const FInputActionValue& Value);
	void HandleDestroyBuild();

///------------------------Input Manager--------------------
public:
	void SetOverlayInputState(EOverlayInputState NewState);
	void EnterBuildPreview(FName BuildingID);
	void ExitBuildPreview(bool bCancel);

	bool IsMenuLikeState() const;
	bool IsBuildPreviewState() const;

	bool DoLineTrace(FHitResult& OutHit);

private:
	void ApplyOverlayInputState();

///---------------------------------------------------------
		

public:

	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

	APlayerController_SB();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, Category = "SB|UI")
	TSubclassOf<class USB_UIManager> UIManagerClass;

	UPROPERTY()
	TObjectPtr<class USB_UIManager> UIManager;

	UFUNCTION(BlueprintCallable, Category = "SB|UI")
	void BP_ResumeFromPause();

	UFUNCTION(BlueprintCallable, Category = "SB|UI")
	void ReturnToPauseFromOptions(UUserWidget* OptionsWidget);

	UPROPERTY(EditDefaultsOnly, Category = "SB|Stamina|Cost")
	float EvasionStaminaCost = 25.f;

	UFUNCTION()
	void OnHealthChanged(float OldValue, float NewValue);

	UFUNCTION()
	void OnStaminaChanged(float OldValue, float NewValue);

private:

	void ToggleFullMap();

	UFUNCTION(Exec)
	void SB_SaveWorld();
	
	UFUNCTION(Exec)
	void SB_LoadWorld();


	UPROPERTY()
	bool bMenuOpen = false;

	UPROPERTY()
	bool bFullMapOpen = false;

// =========================
// Gather Progress System
// =========================

private:

	float GatherStartTime = 0.f;
	float GatherDuration = 0.f;
	bool bGathering = false;

	FTimerHandle GatherUpdateTimer;

	void UpdateGatherUI();

	FTimerHandle FullMapUpdateTimer;
	void UpdateFullMap();


public:

	void StartGatherProgress(float Duration);
	void EndGatherProgress();

// =========================
// Ping System UI
// =========================

private:

	FVector2D CurrentPingUV = FVector2D::ZeroVector;
	bool bHasPing = false;

	UPROPERTY(EditDefaultsOnly, Category = "SB|Ping")
	float PingToggleThreshold = 0.01f;

public:

	void SetPing(const FVector2D& InUV);
	void ClearPing();

	bool HasPing() const { return bHasPing; }
	FVector2D GetPingUV() const { return CurrentPingUV;}

// =========================
// Game Clear UI
// =========================

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SB|UI")
	TSubclassOf<class UUserWidget> GameClearWidgetClass;

	UFUNCTION(BlueprintCallable)
	void ShowGameClearUI(bool bBossKilled);

	UFUNCTION(BlueprintCallable)
	void GoToTitleMenu();

};
