#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuildComponent.generated.h"

class APlayerCharacter_SB;
class UCameraComponent;
class UDataTable;
struct FBuildingDataRow;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STILLBOUND_API UBuildComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuildComponent();

	virtual void BeginPlay() override;

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
	bool IsBuildModeOn = false;

	FTimerHandle BuildPreviewTimerHandle;



	UFUNCTION(BlueprintCallable)
	void BeginBuildMode(FName InBuildingID);

	UFUNCTION(BlueprintCallable)
	void CancelBuildMode();

	bool GetBuildingData(FName InBuildingID, FBuildingDataRow& OutRow) const;

	void SpawnBuildGhost();

	void UpdateBuildPreview();
	void UpdatePreviewTransform();



};
