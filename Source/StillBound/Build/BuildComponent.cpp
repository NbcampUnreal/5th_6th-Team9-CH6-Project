#include "Build/BuildComponent.h"

UBuildComponent::UBuildComponent()
{

	PrimaryComponentTick.bCanEverTick = true;

}

void UBuildComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UBuildComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

