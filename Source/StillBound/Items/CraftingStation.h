#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractionInterface.h"
#include "CraftingStation.generated.h"

class UDataTable;
class APlayerCharacter_SB;

UCLASS()
class STILLBOUND_API ACraftingStation : public AActor, public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	ACraftingStation();

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, Category="Crafting")
	TObjectPtr<UDataTable> RecipeTable;

	UPROPERTY(EditDefaultsOnly, Category = "Crafting")
	FName StationTag = "Workbench";

public:
	virtual void Interact_Implementation(APlayerCharacter_SB* PlayerCharacter) override;

};
