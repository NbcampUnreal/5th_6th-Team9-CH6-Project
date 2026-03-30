#include "Build/BuildComponent.h"
#include "Character/PlayerCharacter_SB.h"
#include "Character/PlayerController_SB.h"
#include "Camera/CameraComponent.h"
#include "Public/Data/BuildingData.h"
#include "Engine/OverlapResult.h"
#include "Inventory/InventoryComponent.h"
#include "Public/Data/ItemData.h"
#include "UI/Build/BuildPreview_IngredientPanel.h"
#include "Landscape.h"
#include "Kismet/GameplayStatics.h"
#include "Items/ItemBase.h"
#include "GuideQuest/GuideQuestSubsystem.h"

#pragma region Helpers

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

TArray<USceneComponent*> UBuildComponent::GetSnapPointsFromActor(AActor* InActor) const
{
	TArray<USceneComponent*> Result;
	if (!InActor) return Result;

	TArray<USceneComponent*> SceneComps;
	InActor->GetComponents<USceneComponent>(SceneComps);

	for (USceneComponent* Comp : SceneComps)
	{
		if (!Comp) continue;

		const FString CompName = Comp->GetName();
		if (CompName.StartsWith(TEXT("Snap_")))
		{
			Result.Add(Comp);
		}
	}

	return Result;
}

void UBuildComponent::AddIgnoredBuildActorsForGroundTrace(FCollisionQueryParams& Params) const
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (!Actor) continue;

		if (Actor->ActorHasTag(TEXT("Build.Roof")) || Actor->ActorHasTag(TEXT("Build.Wall")))
		{
			Params.AddIgnoredActor(Actor);
		}
	}
}

#pragma endregion

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
	BuildGhost->SetWorldRotation(BuildTransform.GetRotation().Rotator());

	FBuildingDataRow Row;
	if (!GetBuildingData(CurrentBuildingID, Row))
	{
		bCanPlace = false;
		ApplyPreviewMaterial(false);
		return;
	}

	const bool bCanPlaceByLocation = CheckCanPlace(Row);
	const bool bHasEnoughCost = HasEnoughBuildCost(Row);

	bCanPlace = bCanPlaceByLocation && bHasEnoughCost;
	ApplyPreviewMaterial(bCanPlace);

	if (Player)
	{
		APlayerController_SB* PC = Cast<APlayerController_SB>(Player->GetController());
		if (PC && PC->UIManager)
		{
			FBuildingDataRow BuildingDataRow;
			if (GetBuildingData(CurrentBuildingID, BuildingDataRow))
			{
				TArray<FBuildPreviewCostUIData> CostUIList;

				if (UInventoryComponent* Inv = Player->GetInventory())
				{
					for (const FBuildCost& Cost : BuildingDataRow.Costs)
					{
						FBuildPreviewCostUIData Data;
						Data.Need = Cost.Count;
						Data.Have = Inv->GetTotalCountByID_ForUI(Cost.ItemID);
						Data.ItemName = FText::FromName(Cost.ItemID);

						if (Inv->ItemDataTable)
						{
							const FItemDataRow* ItemRow = Inv->ItemDataTable->FindRow<FItemDataRow>(Cost.ItemID, TEXT("BuildPreviewCost"));
							if (ItemRow)
							{
								Data.Icon = ItemRow->AssetData.Icon;
								Data.ItemName = ItemRow->TextData.Name;
							}
						}

						CostUIList.Add(Data);
					}
				}

				PC->UIManager->UpdateBuildPreviewPanel(CostUIList);
			}
		}
	}
}

void UBuildComponent::UpdatePreviewTransform()
{
	if (!Player || !Camera) return;

	FBuildingDataRow Row;
	if (!GetBuildingData(CurrentBuildingID, Row)) return;

	const FVector ViewStart = Camera->GetComponentLocation();
	const FVector ViewEnd = ViewStart + Camera->GetForwardVector() * 3000.f;

	FCollisionQueryParams ViewParams;
	ViewParams.AddIgnoredActor(Player);

	FHitResult ViewHit;
	const bool bViewHit = GetWorld()->LineTraceSingleByChannel(
		ViewHit,
		ViewStart,
		ViewEnd,
		ECC_Visibility,
		ViewParams
	);

	if (bViewHit)
	{
		LastPreviewHit = ViewHit;
	}
	else
	{
		LastPreviewHit = FHitResult();
	}

	FVector FinalLocation = bViewHit ? ViewHit.ImpactPoint : ViewEnd;
	FRotator FinalRotation(0.f, CurrentBuildYaw, 0.f);

	FHitResult GroundHit;
	bool bGroundHit = false;

	{
		FCollisionQueryParams GroundParams;
		GroundParams.AddIgnoredActor(Player);

		// 토대/작업대는 지붕, 벽 무시
		if (Row.SnapRule == EBuildSnapRule::GroundOnly ||
			Row.SnapRule == EBuildSnapRule::None)
		{
			AddIgnoredBuildActorsForGroundTrace(GroundParams);
		}

		const FVector GroundTraceStart(FinalLocation.X, FinalLocation.Y, FinalLocation.Z + 5000.f);
		const FVector GroundTraceEnd(FinalLocation.X, FinalLocation.Y, FinalLocation.Z - 5000.f);

		bGroundHit = GetWorld()->LineTraceSingleByChannel(
			GroundHit,
			GroundTraceStart,
			GroundTraceEnd,
			ECC_Visibility,
			GroundParams
		);
	}

	if (bGroundHit)
	{
		LastGroundHit = GroundHit;
	}
	else
	{
		LastGroundHit = FHitResult();
	}

	bSnappedToBuild = false;
	CurrentSnapTargetActor = nullptr;

	switch (Row.SnapRule)
	{
	case EBuildSnapRule::FoundationEdgeOnly:
		if (TrySnapWall(Row, FinalLocation, FinalRotation))
		{
			bSnappedToBuild = true;
		}
		break;

	case EBuildSnapRule::OnTopOfWall:
		if (TrySnapRoof(Row, FinalLocation, FinalRotation))
		{
			bSnappedToBuild = true;
		}
		break;

	case EBuildSnapRule::GroundOnly:
		if (bGroundHit)
		{
			FinalLocation = GroundHit.ImpactPoint;
		}

		FinalLocation.Z += CurrentBuildHeightOffset;
		FinalRotation += Row.PreviewRotationOffset;

		if (TrySnapToNearbyFoundation(FinalLocation))
		{
			bSnappedToBuild = true;
		}
		break;

	case EBuildSnapRule::None:
		if (bGroundHit)
		{
			FinalLocation = GroundHit.ImpactPoint;
		}

		FinalRotation += Row.PreviewRotationOffset;
		break;

	default:
		break;
	}

	BuildTransform.SetLocation(FinalLocation);
	BuildTransform.SetRotation(FQuat(FinalRotation));
}

bool UBuildComponent::CheckCanPlace(const FBuildingDataRow& Row)
{
	switch (Row.SnapRule)
	{
	case EBuildSnapRule::None:
		return CheckFreePlaceOnGround(Row);

	case EBuildSnapRule::GroundOnly:
		return CheckGroundOnlyPlacement(Row);

	case EBuildSnapRule::FoundationEdgeOnly:
		return CheckFoundationEdgePlacement(Row);

	case EBuildSnapRule::OnTopOfWall:
		return CheckRoofPlacement(Row);

	default:
		return false;
	}
}

bool UBuildComponent::CheckGroundOnlyPlacement(const FBuildingDataRow& Row)
{
	//스냅되었으면 지면 검사 없이 허용
	if (bSnappedToBuild)
	{
		if (CheckOverlapAtPreview(Row))
		{
			return false;
		}

		return true;
	}

	// 스냅 안된 경우에만 지면 검사
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

	FVector BoxCenter = Bounds.Origin;
	FVector BoxExtent;

	BoxExtent *= 0.9f;

	if (Row.SnapRule == EBuildSnapRule::FoundationEdgeOnly)
	{
		BoxExtent.X *= 0.55f;
		BoxExtent.Y *= 0.55f;
		BoxExtent.Z *= 0.9f;
	}

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);

	TArray<FOverlapResult> Overlaps;

	const bool bOverlapped = GetWorld()->OverlapMultiByChannel(
		Overlaps, 
		BoxCenter, 
		FQuat::Identity, 
		ECC_WorldStatic, 
		FCollisionShape::MakeBox(BoxExtent), 
		Params);

	if (!bOverlapped) return false;

	for (const FOverlapResult& Result : Overlaps)
	{
		const AActor* OverlapActor = Result.GetActor();
		if (!OverlapActor) continue;

		if (OverlapActor == BuildGhost->GetOwner()) continue;

		// Landscape Ignore
		if (OverlapActor->IsA<ALandscape>()) continue;

		//현재 스냅 대상은 무시
		if (CurrentSnapTargetActor && OverlapActor == CurrentSnapTargetActor) continue;

		//벽 설치 중이면 지붕과의 겹칭은 허용
		if (Row.SnapRule == EBuildSnapRule::FoundationEdgeOnly)
		{
			if (OverlapActor->ActorHasTag(TEXT("Build.Roof")))
			{
				continue;
			}
		}
		// 지붕 설치 중이면 벽/지붕 스냅 대상은 허용
		if (Row.SnapRule == EBuildSnapRule::OnTopOfWall)
		{
			if (OverlapActor->ActorHasTag(TEXT("Build.Wall")) || OverlapActor->ActorHasTag(TEXT("Build.Roof")))
			{
				continue;
			}
		}
		return true;
	}

	return false;
}

bool UBuildComponent::CheckFreePlaceOnGround(const FBuildingDataRow& Row)
{
	if (!LastGroundHit.bBlockingHit)
	{
		return false;
	}

	//기울기 검사
	const FVector HitNormal = LastGroundHit.ImpactNormal;
	const float UpDot = FVector::DotProduct(HitNormal, FVector::UpVector);

	if (UpDot < 0.95)
	{
		return false;
	}

	//겹침 검사
	if (CheckOverlapAtPreview(Row))
	{
		return false;
	}

	return true;
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
	if (!Player) return false;

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

bool UBuildComponent::ConfirmBuild(EBuildFailReason& OutFailReason)
{
	OutFailReason = EBuildFailReason::None;

	if (!bIsBuildModeOn)
	{
		OutFailReason = EBuildFailReason::InvalidPlacement;
		return false;
	}

	FBuildingDataRow Row;
	if (!GetBuildingData(CurrentBuildingID, Row))
	{
		OutFailReason = EBuildFailReason::SpawnFailed;
		return false;
	}

	//위치 불가
	if (!CheckCanPlace(Row))
	{
		OutFailReason = EBuildFailReason::InvalidPlacement;
		return false;
	}

	//재료 부족
	if (!HasEnoughBuildCost(Row))
	{
		OutFailReason = EBuildFailReason::NotEnoughCost;
		return false;
	}

	//재료 차감
	if (!ConsumeBuildCost(Row))
	{
		OutFailReason = EBuildFailReason::NotEnoughCost;
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Player;
	SpawnParams.Instigator = Player;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* Spawned = GetWorld()->SpawnActor<AActor>(Row.BuildActorClass, BuildTransform, SpawnParams);


	if (!Spawned)
	{
		OutFailReason = EBuildFailReason::SpawnFailed;
		return false;
	}

	Spawned->Tags.AddUnique(TEXT("PlacedBuild"));
	Spawned->Tags.AddUnique(CurrentBuildingID);

	//가이드 퀘스트 GameInstance에 넣기
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UGuideQuestSubsystem* QuestSys = GI->GetSubsystem<UGuideQuestSubsystem>())
		{
			//디버깅용 코드
			UE_LOG(LogTemp, Warning, TEXT("[GuideQuest] ReportBuildPlaced called: %s"),
				*CurrentBuildingID.ToString());

			QuestSys->ReportBuildPlaced(CurrentBuildingID, 1);
		}
	}
	//===============

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

void UBuildComponent::AdjustBuildHeight(int32 Direction)
{
	CurrentBuildHeightOffset += Direction * HeightStep;
	CurrentBuildHeightOffset = FMath::Clamp(CurrentBuildHeightOffset, MinBuildHeightOffset, MaxBuildHeightOffset);
}

bool UBuildComponent::TrySnapToNearbyFoundation(FVector& InOutLocation) const
{
	const float SnapSearchRadius = 300.f;
	const float SnapAcceptDistance = 200.f;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);

	const bool bFound = GetWorld()->OverlapMultiByChannel(
		Overlaps,
		InOutLocation,
		FQuat::Identity,
		ECC_WorldDynamic,
		FCollisionShape::MakeSphere(SnapSearchRadius),
		Params
	);

	if (!bFound) return false;

	float BestDistSq = TNumericLimits<float>::Max();
	FVector BestSnapLocation = InOutLocation;
	bool bHasSnap = false;

	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* OverlapActor = Result.GetActor();
		if (!OverlapActor) continue;

		// 태그로 토대 판별
		if (!OverlapActor->ActorHasTag(TEXT("Build.Foundation"))) continue;

		//토대 전용 스냅포인트만 허용
		const TArray<USceneComponent*> SnapPoints = GetSnapPointsByPrefix(OverlapActor, TEXT("Snap_Foundation_"));

		for (USceneComponent* SnapPoint : SnapPoints)
		{
			if (!SnapPoint) continue;

			const FVector SnapLoc = SnapPoint->GetComponentLocation();
			const float DistSq = FVector::DistSquared(SnapLoc, InOutLocation);

			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestSnapLocation = SnapLoc;
				bHasSnap = true;
			}
		}
	}

	if (bHasSnap && BestDistSq <= FMath::Square(SnapAcceptDistance))
	{
		InOutLocation = BestSnapLocation;
		return true;
	}

	return false;
}

bool UBuildComponent::CheckFoundationEdgePlacement(const FBuildingDataRow& Row)
{
	if (!bSnappedToBuild) return false;

	if (CheckOverlapAtPreview(Row)) return false;

	return true;
}

bool UBuildComponent::TrySnapWall(const FBuildingDataRow& Row, FVector& InOutLocation, FRotator& OutRotation) 
{
	if (!Player) return false;

	const float SnapSearchRadius = 350.f;
	const float SnapAcceptDistance = 180.f;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);
	
	const bool bFound = GetWorld()->OverlapMultiByChannel(
		Overlaps,
		InOutLocation,
		FQuat::Identity,
		ECC_WorldDynamic,
		FCollisionShape::MakeSphere(SnapSearchRadius),
		Params);

	if (!bFound) return false;

	float BestDistSq = TNumericLimits<float>::Max();
	FVector BestSnapLocation = InOutLocation;
	FRotator BestSnapRotation = OutRotation;
	AActor* BestSnapActor = nullptr;
	bool bHasSnap = false;

	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* OverlapActor = Result.GetActor();
		if (!OverlapActor) continue;

		TArray<USceneComponent*> SnapPoints;

		// 토대에 붙는벽
		if (OverlapActor->ActorHasTag(TEXT("Build.Foundation")))
		{
			SnapPoints = GetSnapPointsByPrefix(OverlapActor, TEXT("Snap_Wall_"));
		}
		// 벽 위에 벽
		else if (OverlapActor->ActorHasTag(TEXT("Build.Wall")))
		{
			SnapPoints = GetSnapPointsByPrefix(OverlapActor, TEXT("Snap_WallTop"));
		}
		else
		{
			continue;
		}

		for (USceneComponent* SnapPoint : SnapPoints)
		{
			if (!SnapPoint) continue;

			const FVector SnapLoc = SnapPoint->GetComponentLocation();
			const float DistSq = FVector::DistSquared(SnapLoc, InOutLocation);

			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestSnapLocation = SnapLoc;

				BestSnapRotation = SnapPoint->GetComponentRotation() + Row.PreviewRotationOffset;
				BestSnapActor = OverlapActor;
				bHasSnap = true;
			}
		}
	}

	if (bHasSnap && BestDistSq <= FMath::Square(SnapAcceptDistance))
	{
		InOutLocation = BestSnapLocation;
		OutRotation = BestSnapRotation;
		CurrentSnapTargetActor = BestSnapActor;
		return true;
	}

	CurrentSnapTargetActor = nullptr;
	return false;
}

TArray<USceneComponent*> UBuildComponent::GetSnapPointsByPrefix(AActor* InActor, const FString& Prefix) const
{
	TArray<USceneComponent*> Result;
	if (!InActor) return Result;

	TArray<USceneComponent*> SceneComponents;
	InActor->GetComponents<USceneComponent>(SceneComponents);

	for (USceneComponent* Comp : SceneComponents)
	{
		if (!Comp) continue;

		const FString CompName = Comp->GetName();
		if (CompName.StartsWith(Prefix))
		{
			Result.Add(Comp);
		}
	}

	return Result;
}

void UBuildComponent::AddBuildRotation(float DeltaYaw)
{
	CurrentBuildYaw += DeltaYaw;

	CurrentBuildYaw = FMath::Fmod(CurrentBuildYaw, 360.f);
}

bool UBuildComponent::CheckRoofPlacement(const FBuildingDataRow& Row)
{
	if (!bSnappedToBuild) return false;

	if (CheckOverlapAtPreview(Row)) return false;

	return true;

}

bool UBuildComponent::TrySnapRoof(const FBuildingDataRow& Row, FVector& InOutLocation, FRotator& OutRotation)
{
	if (!Player)
	{
		CurrentSnapTargetActor = nullptr;
		return false;
	}

	AActor* HitActor = LastPreviewHit.GetActor();
	if (!HitActor)
	{
		CurrentSnapTargetActor = nullptr;
		return false;
	}

	const bool bIsWall = HitActor->ActorHasTag(TEXT("Build.Wall"));
	const bool bIsRoof = HitActor->ActorHasTag(TEXT("Build.Roof"));

	if (!bIsWall && !bIsRoof)
	{
		CurrentSnapTargetActor = nullptr;
		return false;
	}

	const TArray<USceneComponent*> SnapPoints =
		GetSnapPointsByPrefix(HitActor, TEXT("Snap_Roof_"));

	if (SnapPoints.Num() == 0)
	{
		CurrentSnapTargetActor = nullptr;
		return false;
	}

	float BestDistSq = TNumericLimits<float>::Max();
	USceneComponent* BestSnapPoint = nullptr;

	for (USceneComponent* SnapPoint : SnapPoints)
	{
		if (!SnapPoint) continue;

		const FVector SnapLoc = SnapPoint->GetComponentLocation();
		const float DistSq = FVector::DistSquared(SnapLoc, InOutLocation);

		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestSnapPoint = SnapPoint;
		}
	}

	if (!BestSnapPoint)
	{
		CurrentSnapTargetActor = nullptr;
		return false;
	}

	InOutLocation = BestSnapPoint->GetComponentLocation();
	OutRotation = BestSnapPoint->GetComponentRotation() + Row.PreviewRotationOffset;
	CurrentSnapTargetActor = HitActor;

	UE_LOG(LogTemp, Warning, TEXT("[RoofSnap] Target=%s  SnapPoint=%s  Loc=%s"),
		*GetNameSafe(HitActor),
		*GetNameSafe(BestSnapPoint),
		*InOutLocation.ToString());

	return true;
}

