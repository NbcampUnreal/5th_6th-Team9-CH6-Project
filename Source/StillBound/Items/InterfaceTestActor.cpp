// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InterfaceTestActor.h"

// Sets default values
AInterfaceTestActor::AInterfaceTestActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	SetRootComponent(Mesh);

}

void AInterfaceTestActor::BeginPlay()
{
	Super::BeginPlay();

	InteractableData = InsntanceInteractableData;
	
}


void AInterfaceTestActor::BeginFocus()
{
	if (Mesh)
	{
		Mesh->SetRenderCustomDepth(true);
	}
}

void AInterfaceTestActor::EndFocus()
{
	if (Mesh)
	{
		Mesh->SetRenderCustomDepth(false);
	}
}

void AInterfaceTestActor::BeginInteract()
{
	UE_LOG(LogTemp, Warning, TEXT("Calling BeginInteract override on interface test actor."))
}

void AInterfaceTestActor::EndInteract()
{
	UE_LOG(LogTemp, Warning, TEXT("Calling EndInteract override on interface test actor."))
}

void AInterfaceTestActor::Interact(APlayerCharacter_SB* PlayerCharacter)
{
	UE_LOG(LogTemp, Warning, TEXT("Calling Interact override on interface test actor."))
}

