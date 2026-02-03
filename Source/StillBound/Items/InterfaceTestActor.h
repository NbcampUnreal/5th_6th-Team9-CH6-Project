// Fill out your copyright notice in the Description page of Project Settings.

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
	// Sets default values for this actor's properties
	AInterfaceTestActor();

protected:
	UPROPERTY(EditAnywhere, Category="Test Actor")
	UStaticMeshComponent* Mesh;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void BeginFocus();
	virtual void EndFocus();
	virtual void BeginInteract();
	virtual void EndInteract();
	virtual void Interact();

};
