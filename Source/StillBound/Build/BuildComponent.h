#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuildComponent.generated.h"

class APlayerCharacter_SB;
class UCameraComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STILLBOUND_API UBuildComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuildComponent();

	TObjectPtr<UCameraComponent> Camera;

	TObjectPtr<APlayerCharacter_SB> Player;

	TObjectPtr<UStaticMeshComponent> BuildGhost;

	FTransform BuildTransform;

	bool IsBuildModeOn;
	bool DoOnce = true;

	void ToggleBuildMode();
	void BuildCycle();
	void SpawnBuildGhost();

	void UpdateBuildPreview();

protected:
	virtual void BeginPlay() override;



public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
