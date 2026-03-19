// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractionInterface.h"
#include "SavePointActor.generated.h"

UCLASS()
class STILLBOUND_API ASavePointActor : public AActor, public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASavePointActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 세이브 포인트 외관
	UPROPERTY(EditAnywhere, Category = "SavePoint")
	UStaticMeshComponent* Mesh;

	// UI에 띄울 상호작용
	UPROPERTY(EditAnywhere, Category = "Interaction")
	FInteractableData InteractableData;

public:	
	// Called every frame
	virtual void BeginFocus_Implementation() override;
	virtual void EndFocus_Implementation() override;
	virtual void BeginInteract_Implementation() override;
	virtual void EndInteract_Implementation() override;

	virtual void Interact_Implementation(APlayerCharacter_SB* PlayerCharacter) override;
	virtual FInteractableData GetInteractableData_Implementation() override;
};
