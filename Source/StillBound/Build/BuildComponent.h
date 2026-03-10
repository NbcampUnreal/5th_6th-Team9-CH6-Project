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

	TObjectPtr<UCameraComponent> Camera;

	TObjectPtr<APlayerCharacter_SB> Player;

	TObjectPtr<UStaticMeshComponent> BuildGhost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build")
	TObjectPtr<UDataTable> BuildingDataTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	FName CurrentBuildingID = NAME_None;

	FTransform BuildTransform;

	FTimerHandle  CycleHandle;

	UFUNCTION(BlueprintCallable)
	void BeginBuildMode(FName InBuildingID);

	UFUNCTION(BlueprintCallable)
	void CancelBuildMode();

	bool GetBuildingData(FName InBuildingID, FBuildingDataRow& OutRow) const;

	bool IsBuildModeOn;
	bool DoOnce = true;

	//void ToggleBuildMode();
	void BuildCycle();
	void SpawnBuildGhost();

	void UpdateBuildPreview();

protected:
	virtual void BeginPlay() override;

};
