#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuildComponent.generated.h"

class APlayerCharacter_SB;
class UCameraComponent;
class UDataTable;
struct FBuildingDataRow;

UENUM(BlueprintType)
enum class EBuildFailReason : uint8
{
	None,
	NotEnoughCost,
	InvalidPlacement,
	SpawnFailed
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STILLBOUND_API UBuildComponent : public UActorComponent
{
	GENERATED_BODY()

///===============================================================================
/// PROPERTIES & VARIABLES
///===============================================================================
public:
	UPROPERTY()
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY()
	TObjectPtr<APlayerCharacter_SB> Player;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> BuildGhost;

	UPROPERTY()
	FTransform BuildTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	TObjectPtr<UDataTable> BuildingDataTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	FName CurrentBuildingID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	bool bIsBuildModeOn = false;

	FTimerHandle BuildPreviewTimerHandle;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Build")
	bool bCanPlace = false;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Preview")
	TObjectPtr<UMaterialInterface> ValidPreviewMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Preview")
	TObjectPtr<UMaterialInterface> InvalidPreviewMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	FHitResult LastPreviewHit;

	UPROPERTY()
	FText LastBuildPreviewStateMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Build")
	float CurrentBuildHeightOffset = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Build")
	float MinBuildHeightOffset = -300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Build")
	float MaxBuildHeightOffset = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Build")
	float HeightStep = 50.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	bool bSnappedToFoundation = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	TObjectPtr<AActor> CurrentSnappedAcotr = nullptr;

///===============================================================================
/// FUNCTIONS
///===============================================================================
public:	
	UBuildComponent();

	virtual void BeginPlay() override;


	UFUNCTION(BlueprintCallable)
	void BeginBuildMode(FName InBuildingID);

	UFUNCTION(BlueprintCallable)
	void CancelBuildMode();

	UFUNCTION(BlueprintCallable)
	bool ConfirmBuild(EBuildFailReason& OutFailReason);

	UFUNCTION(BlueprintCallable)
	void HandleBuildCancel();

	bool ConsumeBuildCost(const FBuildingDataRow& Row);

	bool GetBuildingData(FName InBuildingID, FBuildingDataRow& OutRow) const;

	void SpawnBuildGhost();

	void UpdateBuildPreview();
	void UpdatePreviewTransform();

	bool CheckCanPlace(const FBuildingDataRow& Row);
	bool CheckGroundOnlyPlacement(const FBuildingDataRow& Row);
	bool CheckOverlapAtPreview(const FBuildingDataRow& Row) const;
	void ApplyPreviewMaterial(bool bInCanPlace);

	bool HasEnoughBuildCost(const FBuildingDataRow& Row) const;
	
	void AdjustBuildHeight(int32 Direction);

	bool TrySnapToNearbyFoundation(FVector& InOutLocation);

	TArray<USceneComponent*> GetSnapPointsFromActor(AActor* InActor) const;
};
