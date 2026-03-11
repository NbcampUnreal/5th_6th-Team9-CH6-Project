#include "Build/BuildComponent.h"
#include "Character/PlayerCharacter_SB.h"
#include "Camera/CameraComponent.h"
#include "Public/Data/BuildingData.h"

bool UBuildComponent::GetBuildingData(FName InBuildingID, FBuildingDataRow& OutRow) const
{
	if (!BuildingDataTable || InBuildingID.IsNone())
	{
		return false;
	}

	const FBuildingDataRow* Found = BuildingDataTable->FindRow<FBuildingDataRow>(InBuildingID, TEXT("BuildLookup"));
	if (!Found)
	{
		UE_LOG(LogTemp, Error, TEXT("[Build] BuildingData row not found: %s"), *InBuildingID.ToString());
		return false;
	}

	OutRow = *Found;
	return true;
}

UBuildComponent::UBuildComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

}

void UBuildComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<APlayerCharacter_SB>(GetOwner());
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("]BuildComponent] Player is not valid"));
		return;
	}

	Camera = Player->FindComponentByClass<UCameraComponent>();

	if (!Camera)
	{
		UE_LOG(LogTemp, Error, TEXT("[BuildComponent] Camera not found"));
	}
}

void UBuildComponent::BeginBuildMode(FName InBuildingID)
{
	if (!Player || !Camera)
	{
		UE_LOG(LogTemp, Error, TEXT("[Build] BeginBuildMode failed: Player or Camera invalid"));
		return;
	}

	if (InBuildingID.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[Build] BeginBuildMode failed: BuildingID is None"));
		return;
	}

	CurrentBuildingID = InBuildingID;
	IsBuildModeOn = true;;

	GetWorld()->GetTimerManager().ClearTimer(BuildPreviewTimerHandle);

	if (BuildGhost)
	{
		BuildGhost->DestroyComponent();
		BuildGhost = nullptr;
	}

	UpdateBuildPreview();

	GetWorld()->GetTimerManager().SetTimer(BuildPreviewTimerHandle, this, &ThisClass::UpdateBuildPreview, 0.02f, true);

}

void UBuildComponent::CancelBuildMode()
{
	IsBuildModeOn = false;
	CurrentBuildingID = NAME_None;

	GetWorld()->GetTimerManager().ClearTimer(BuildPreviewTimerHandle);

	if (BuildGhost)
	{
		BuildGhost->DestroyComponent();
		BuildGhost = nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Build] CancelBuildMode"));
}

void UBuildComponent::SpawnBuildGhost()
{
	if (!Player || CurrentBuildingID.IsNone()) return;

	FBuildingDataRow Row;
	if (!GetBuildingData(CurrentBuildingID, Row)) return;

	if (!Row.PreviewMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("[Build] BeginBuildMode failed: Player or Camera invalid"));
		return;
	}

	BuildGhost = NewObject<UStaticMeshComponent>(Player);
	if (!BuildGhost) return;

	BuildGhost->RegisterComponent();
	BuildGhost->AttachToComponent(Player->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);

	BuildGhost->SetStaticMesh(Row.PreviewMesh);
	BuildGhost->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BuildGhost->SetGenerateOverlapEvents(false);

	BuildGhost->SetWorldTransform(BuildTransform);
}

void UBuildComponent::UpdateBuildPreview()
{
	if (!IsBuildModeOn) return;

	if (!BuildGhost)
	{
		SpawnBuildGhost();
		if (!BuildGhost)
		{
			return;
		}
	}

	UpdatePreviewTransform();

	BuildGhost->SetWorldLocation(BuildTransform.GetLocation());
	BuildGhost->SetWorldRotation(FRotator::ZeroRotator);
}

void UBuildComponent::UpdatePreviewTransform()
{
	if (!Player || !Camera) return;

	const FVector StartLocation = Camera->GetComponentLocation();
	const FVector EndLocation = StartLocation + Camera->GetForwardVector() * 1000.f;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);

	if (bHit)
	{
		BuildTransform.SetLocation(HitResult.ImpactPoint);
	}
	else
	{
		BuildTransform.SetLocation(EndLocation);
	}

	BuildTransform.SetRotation(FQuat(FRotator::ZeroRotator));

	/// LineTrace Debugging
	// DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Yellow, false, 0.f, 0, 1.f);
}