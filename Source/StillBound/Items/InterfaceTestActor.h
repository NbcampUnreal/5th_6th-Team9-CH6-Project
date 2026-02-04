#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractionInterface.h"
#include "InterfaceTestActor.generated.h"

UCLASS()
class STILLBOUND_API AInterfaceTestActor : public AActor, public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	AInterfaceTestActor();

protected:
	UPROPERTY(EditAnywhere, Category="Test Actor")
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditInstanceOnly, Category="Text Actor")
	FInteractableData InsntanceInteractableData;

	virtual void BeginPlay() override;

	virtual void BeginFocus();
	virtual void EndFocus();
	virtual void BeginInteract();
	virtual void EndInteract();
	virtual void Interact(APlayerCharacter_SB* PlayerCharacter);

};
