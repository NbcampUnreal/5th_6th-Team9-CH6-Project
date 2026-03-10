#include "Build/BuildComponent.h"
#include "Character/PlayerCharacter_SB.h"
#include "Camera/CameraComponent.h"
#include "Public/Data/BuildingData.h"

UBuildComponent::UBuildComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

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

/*
void UBuildComponent::ToggleBuildMode()
{
	if (!IsBuildModeOn)
	{
		Player->GetWorldTimerManager().SetTimer(CycleHandle, this, &ThisClass::UpdateBuildPreview, 0.01f, true);
		IsBuildModeOn = true;
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(CycleHandle);
		if (BuildGhost)
		{
			Player->DestroyActorComponent(BuildGhost);
		}
		DoOnce = true;
		IsBuildModeOn = false;
	}
}
*/

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

	UStaticMesh* LoadedMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("/Game/ItsMeBroBaseBuildingAssets/Build/FullBuild.FullBuild")));

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

void UBuildComponent::BeginBuildMode(FName InBuildingID)
{
	CurrentBuildingID = InBuildingID;

	if (!IsBuildModeOn)
	{
		IsBuildModeOn = true;
		DoOnce = true;
	}
}

void UBuildComponent::CancelBuildMode()
{
	IsBuildModeOn = false;
	DoOnce = true;
	CurrentBuildingID = NAME_None;

	if (BuildGhost)
	{
		BuildGhost->DestroyComponent();
		BuildGhost = nullptr;
	}
}

bool UBuildComponent::GetBuildingData(FName InBuildingID, FBuildingDataRow& OutRow) const
{
	if (!BuildingDataTable || InBuildingID.IsNone())
	{
		return false;
	}

	const FBuildingDataRow* Found = BuildingDataTable->FindRow<FBuildingDataRow>(InBuildingID, TEXT("BuildLookup"));
	if (!Found)
	{
		return false;
	}

	OutRow = *Found;
	return true;
}