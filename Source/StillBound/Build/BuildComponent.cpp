#include "Build/BuildComponent.h"
#include "Character/PlayerCharacter_SB.h"
#include "Camera/CameraComponent.h"
#include "Public/Data/BuildingData.h"
#include "Engine/OverlapResult.h"
#include "Inventory/InventoryComponent.h"

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
	bIsBuildModeOn = true;;

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
	bIsBuildModeOn = false;
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
	if (!bIsBuildModeOn) return;

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

	FBuildingDataRow Row;
	if (!GetBuildingData(CurrentBuildingID, Row))
	{
		bCanPlace = false;
		ApplyPreviewMaterial(false);
		return;
	}

	bCanPlace = CheckCanPlace(Row) /* && HasEnoughBuildCost(Row) */ ;
	ApplyPreviewMaterial(bCanPlace);
}

void UBuildComponent::UpdatePreviewTransform()
{
	if (!Player || !Camera) return;

	const FVector StartLocation = Camera->GetComponentLocation();
	const FVector EndLocation = StartLocation + Camera->GetForwardVector() * 1000.f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(LastPreviewHit, StartLocation, EndLocation, ECC_Visibility, Params);

	if (bHit)
	{
		BuildTransform.SetLocation(LastPreviewHit.ImpactPoint);
	}
	else
	{
		BuildTransform.SetLocation(EndLocation);
		LastPreviewHit = FHitResult();
	}

	BuildTransform.SetRotation(FQuat(FRotator::ZeroRotator));

	/// LineTrace Debugging
	// DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Yellow, false, 0.f, 0, 1.f);
}

bool UBuildComponent::CheckCanPlace(const FBuildingDataRow& Row)
{
	switch (Row.SnapRule)
	{
	case EBuildSnapRule::None:

		//일단은 GroundOnly취급함. 자유배치면 단순 겹침만 검사
		return CheckGroundOnlyPlacement(Row);

	case EBuildSnapRule::GroundOnly:
		return CheckGroundOnlyPlacement(Row);

	case EBuildSnapRule::FoundationEdgeOnly:
		// 추후 구현예정
		return false;

	case EBuildSnapRule::OnTopOfWall:
		//추후 구현예정
		return false;

	default:
		return false;
	}
}

bool UBuildComponent::CheckGroundOnlyPlacement(const FBuildingDataRow& Row)
{
	if (!LastPreviewHit.bBlockingHit)
	{
		return false;
	}

	AActor* HitActor = LastPreviewHit.GetActor();
	if (!HitActor)
	{
		return false;
	}

	if (CheckOverlapAtPreview(Row))
	{
		return false;
	}
	
	return true;
}

bool UBuildComponent::CheckOverlapAtPreview(const FBuildingDataRow& Row) const
{
	if (!BuildGhost) return true;

	const FBoxSphereBounds Bounds = BuildGhost->CalcBounds(BuildGhost->GetComponentTransform());

	const FVector BoxCenter = Bounds.Origin;
	const FVector BoxExtent = Bounds.BoxExtent * 0.95f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);

	TArray<FOverlapResult> Overlaps;

	const bool bOverlapped = GetWorld()->OverlapMultiByChannel(Overlaps, BoxCenter, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeBox(BoxExtent), Params);

	if (!bOverlapped) return false;

	for (const FOverlapResult& Result : Overlaps)
	{
		const AActor* OverlapActor = Result.GetActor();
		if (!OverlapActor) continue;

		if (OverlapActor == BuildGhost->GetOwner()) continue;

		return true;
	}

	return false;
}

void UBuildComponent::ApplyPreviewMaterial(bool bInCanPlace)
{
	if (!BuildGhost) return;

	UMaterialInterface* TargetMaterial = bInCanPlace ? ValidPreviewMaterial : InvalidPreviewMaterial;
	if (!TargetMaterial) return;

	const int32 MaterialCount = BuildGhost->GetNumMaterials();
	for (int32 i = 0; i < MaterialCount; ++i)
	{
		BuildGhost->SetMaterial(i, TargetMaterial);
	}
}

bool UBuildComponent::HasEnoughBuildCost(const FBuildingDataRow& Row) const
{
	if (Player) return false;

	UInventoryComponent* Inv = Player->GetInventory();
	if (!Inv) return false;

	for (const FBuildCost& Cost : Row.Costs)
	{
		const int32 Have = Inv->GetTotalCountByID_ForUI(Cost.ItemID);
		if (Have < Cost.Count)
		{
			return false;
		}
	}
	
	return true;
}

bool UBuildComponent::ConfirmBuild()
{
	if (!bIsBuildModeOn) return false;

	if (!bCanPlace)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Build] ConfirmBuild failed: bCanPlace is false"));
		return false;
	}
	
	if (!Player || CurrentBuildingID.IsNone()) return false;

	FBuildingDataRow Row;
	if (!GetBuildingData(CurrentBuildingID, Row)) return false;

	if (!Row.BuildActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Build] BuildActorClass is null for %s"), *CurrentBuildingID.ToString());
		return false;
	}

	if (!ConsumeBuildCost(Row))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Build] ConfirmBuild failed: cost consume failed"));
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Player;
	SpawnParams.Instigator = Player;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* Spawned = GetWorld()->SpawnActor<AActor>(Row.BuildActorClass, BuildTransform, SpawnParams);

	if (!Spawned)
	{
		UE_LOG(LogTemp, Error, TEXT("[Build] SpawnActor failed for %s"), *CurrentBuildingID.ToString());
		return false;
	}

	//만약 한번 설치하고 프리뷰유지하고싶지않으면 false;
	return true;
}

void UBuildComponent::HandleBuildCancel()
{
	CancelBuildMode();
}

bool UBuildComponent::ConsumeBuildCost(const FBuildingDataRow& Row)
{
	if (!Player) return false;

	UInventoryComponent* Inv = Player->GetInventory();
	if (!Inv) return false;

	for (const FBuildCost& Cost : Row.Costs)
	{
		const int32 Have = Inv->GetTotalCountByID_ForUI(Cost.ItemID);
		if (Have < Cost.Count)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Build] Not enough cost: %s Need=%d Have=%d"), *Cost.ItemID.ToString(), Cost.Count, Have);
			return false;
		}
	}

	for (const FBuildCost& Cost : Row.Costs)
	{
		const bool bConsumed = Inv->ConsumeByID(Cost.ItemID, Cost.Count);
		if (!bConsumed)
		{
			UE_LOG(LogTemp, Error, TEXT("[Build] ConsumeByID failed: %s x%d"), *Cost.ItemID.ToString(), Cost.Count);
			return false;
		}
	}
	return true;
}