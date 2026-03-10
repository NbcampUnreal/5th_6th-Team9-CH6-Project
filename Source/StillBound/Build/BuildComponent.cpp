#include "Build/BuildComponent.h"
#include "Character/PlayerCharacter_SB.h"
#include "Camera/CameraComponent.h"

UBuildComponent::UBuildComponent()
{

	PrimaryComponentTick.bCanEverTick = true;

}

void UBuildComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	BuildCycle();
}

void UBuildComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<APlayerCharacter_SB>(GetOwner());
	
	if (Player)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("BuildComponent Cast has succeeded."));
	}
}

void UBuildComponent::ToggleBuildMode()
{
	if (!IsBuildModeOn)
	{
		IsBuildModeOn = true;
	}
	else
	{
		IsBuildModeOn = false;
	}
}

void UBuildComponent::BuildCycle()
{
	FVector StartLocation = Camera->GetComponentLocation() + Camera->GetForwardVector() * 350.f;
	FVector EndLocation = Camera->GetComponentLocation() + Camera->GetForwardVector() * 1000.f;

	FHitResult HitResult;
	
	FCollisionQueryParams CollisionParameters;

	CollisionParameters.AddIgnoredActor(GetOwner());
	
	bool Hit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility, CollisionParameters);

	if (Hit)
	{
		BuildTransform.SetLocation(HitResult.ImpactPoint);
	}
	else
	{
		BuildTransform.SetLocation(HitResult.TraceEnd);
	}

	///LineTrace Debugging
	//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.0f, 0, 2.0f);
}

void UBuildComponent::SpawnBuildGhost()
{
	BuildGhost = Cast<UStaticMeshComponent>(Player->AddComponentByClass(UStaticMeshComponent::StaticClass(), false, BuildTransform, false));

	UStaticMesh* LoadedMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("원래라면 매시주소레퍼런스")));

	BuildGhost->SetStaticMesh(LoadedMesh);
	BuildGhost->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UBuildComponent::UpdateBuildPreview()
{
	if (DoOnce)
	{
		SpawnBuildGhost();
		DoOnce = false;
	}
	BuildCycle();

	if (BuildGhost)
	{
		BuildGhost->SetWorldLocation(BuildTransform.GetLocation());
		BuildGhost->SetWorldRotation(FRotator::ZeroRotator);
	}
}

