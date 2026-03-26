

#include "AI/EnemyCharacter.h"
#include "AI/EnemyAIController.h"
#include "EnemyVisualRow.h"
#include "Engine/DataTable.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/AIAttributeSet.h"
#include "AIController.h"
#include "Items/Pickup.h"
#include "Items/ItemBase.h"
#include "Data/ItemData.h"
#include "UI/DamageNumberActor.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	AttributeSetClassForInitStats = UAIAttributeSet::StaticClass();

	AIControllerClass = AEnemyAIController::StaticClass();

	// ===== Alert Anchor =====
	AlertAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("AlertAnchor"));
	AlertAnchor->SetupAttachment(GetRootComponent());

	// ===== Alert Widget =====
	AlertWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("AlertWidget"));
	AlertWidgetComponent->SetupAttachment(AlertAnchor);
	AlertWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	AlertWidgetComponent->SetVisibility(false);
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

	InitialSpawnLocation = GetActorLocation();
	InitialSpawnRotation = GetActorRotation();

	ApplyVisualFromDataTable();

	if (AlertAnchor && GetMesh())
	{
		float HeadHeight = GetMesh()->Bounds.BoxExtent.Z;

		AlertAnchor->SetRelativeLocation(
			FVector(0.f, 0.f, HeadHeight + 20.f)
		);
	}

    AIController = Cast<AEnemyAIController>(GetController());

	if (!AIController)
	{
		return;
	}

	BlackboardComp = AIController->GetBlackboardComponent();
	if (!BlackboardComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyCharacter] BlackboardComp is null (BT/BB init failed?)"));
		return;
	}

	BlackboardComp->SetValueAsBool(TEXT("bIsRangedEnemy"), IsRangedEnemy());
	BlackboardComp->SetValueAsFloat(TEXT("AttackRange"), GetPreferredAttackRange());

	UE_LOG(LogTemp, Warning, TEXT("[EnemyCharacter] Blackboard Ready: %s"), *BlackboardComp->GetName());
}

void AEnemyCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyVisualFromDataTable();
}

void AEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AEnemyCharacter::ApplyVisualFromDataTable()
{
	if (!EnemyVisualDataTable)
	{
		return;
	}

	if (EnemyId <= 0)
	{
		return;
	}

	const FEnemyVisualRow* FoundRow = nullptr;
	FName FoundRowName = NAME_None;

	const TMap<FName, uint8*>& RowMap = EnemyVisualDataTable->GetRowMap();
	for (const TPair<FName, uint8*>& Pair : RowMap)
	{
		const FEnemyVisualRow* Row = reinterpret_cast<const FEnemyVisualRow*>(Pair.Value);
		if (!Row) continue;

		if (Row->EnemyId == EnemyId)
		{
			FoundRow = Row;
			FoundRowName = Pair.Key;
			break;
		}
	}

	if (!FoundRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyCharacter] No row matched EnemyId=%d"), EnemyId);
		return;
	}

	if (FoundRow->Mesh)
	{
		GetMesh()->SetSkeletalMesh(FoundRow->Mesh);
	}

	if (FoundRow->AnimClass)
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(FoundRow->AnimClass);
	}

	AttackMontage = FoundRow->AttackMontage;
	DeathMontage = FoundRow->DeathMontage;
	GetHitMontage = FoundRow->GetHitMontage;

	AttackType = FoundRow->AttackType;
	PreferredAttackRange = FoundRow->PreferredAttackRange;

	if (UCapsuleComponent* Cap = GetCapsuleComponent())
	{
		const bool bHasCapsuleSize =
			(FoundRow->CapsuleRadius > 0.f && FoundRow->CapsuleHalfHeight > 0.f);

		if (bHasCapsuleSize)
		{
			Cap->SetCapsuleSize(FoundRow->CapsuleRadius, FoundRow->CapsuleHalfHeight, true);
		}
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		const float SafeScale = (FoundRow->MeshScale > 0.f) ? FoundRow->MeshScale : 1.f; MeshComp->SetRelativeScale3D(FVector(SafeScale));

		MeshComp->SetRelativeLocation(FoundRow->MeshRelativeLocation);
	}
}

void AEnemyCharacter::BeginDeathState()
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->StopMovementImmediately();
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AEnemyCharacter::SpawnDropItems()
{


	if (!ItemDataTable || !PickupClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (const FEnemyDropItem& Drop : DropItems)
	{
		if (Drop.ItemID.IsNone())
		{
			continue;
		}

		if (Drop.DropChance < 1.0f)
		{
			const float Roll = FMath::FRand();
			if (Roll > Drop.DropChance)
			{
				continue;
			}
		}

		const int32 Count = FMath::RandRange(Drop.MinCount, Drop.MaxCount);
		if (Count <= 0)
		{
			continue;
		}

		FItemDataRow* ItemRow = ItemDataTable->FindRow<FItemDataRow>(Drop.ItemID, TEXT("EnemyDrop"));
		if (!ItemRow)
		{
			UE_LOG(LogTemp, Warning, TEXT("[EnemyDrop] ItemRow not found. ItemID=%s"), *Drop.ItemID.ToString());
			continue;
		}

		UItemBase* NewItem = NewObject<UItemBase>(this);
		if (!NewItem)
		{
			continue;
		}

		NewItem->ID = ItemRow->ID;
		NewItem->ItemType = ItemRow->ItemType;
		NewItem->ItemQuality = ItemRow->ItemQuality;
		NewItem->ItemStatistics = ItemRow->ItemStatistics;
		NewItem->TextData = ItemRow->TextData;
		NewItem->NumericData = ItemRow->NumericData;
		NewItem->AssetData = ItemRow->AssetData;
		NewItem->PickupActorClass = ItemRow->PickupActorClass;
		NewItem->Quantity = Count;

		const FVector SpawnOffset(
			FMath::RandRange(-50.f, 50.f),
			FMath::RandRange(-50.f, 50.f),
			30.f
		);

		const FVector SpawnLocation = GetActorLocation() + SpawnOffset;
		const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APickup* SpawnedPickup = World->SpawnActor<APickup>(PickupClass, SpawnTransform, SpawnParams);
		if (SpawnedPickup)
		{
			SpawnedPickup->InitializeDrop(NewItem, Count);
		}
	}
}

void AEnemyCharacter::HandleDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	BeginDeathState();
	SpawnDropItems();

	const float DeathHideDelay = FMath::Max(GetDeathMontageLength() - 1.5f, 0.0f);

	if (DeathHideDelay > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			HideBodyTimerHandle,
			this,
			&AEnemyCharacter::HideDeadBody,
			DeathHideDelay,
			false
		);
	}
	else
	{
		HideDeadBody();
	}

	GetWorldTimerManager().SetTimer(
		RespawnTimerHandle,
		this,
		&AEnemyCharacter::RespawnEnemy,
		RespawnTime,
		false
	);
}

void AEnemyCharacter::SetEnemyId(int32 NewId)
{
	EnemyId = NewId;
	ApplyVisualFromDataTable();
}

void AEnemyCharacter::SpawnDamageText(float Damage)
{
	if (!DamageNumberClass) return;

	FVector SpawnLocation = GetActorLocation();

	SpawnLocation.X += FMath::RandRange(-40.f, 40.f);
	SpawnLocation.Y += FMath::RandRange(-40.f, 40.f);
	SpawnLocation.Z += FMath::RandRange(10.f, 30.f);

	ADamageNumberActor* Actor =
		GetWorld()->SpawnActor<ADamageNumberActor>(
			DamageNumberClass,
			SpawnLocation,
			FRotator::ZeroRotator
		);

	if (Actor)
	{
		Actor->InitDamage(Damage);
	}
}

void AEnemyCharacter::ShowAlert()
{
	if (!AlertWidgetComponent)
	{
		return;
	}

	AlertWidgetComponent->SetVisibility(true);

	GetWorldTimerManager().SetTimer(
		AlertHideTimer,
		this,
		&AEnemyCharacter::HideAlert,
		1.0f,
		false
	);
}

void AEnemyCharacter::HideAlert()
{
	if (AlertWidgetComponent)
	{
		AlertWidgetComponent->SetVisibility(false);
	}
}

void AEnemyCharacter::DisableEnemyForRespawn()
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
		AIC->UnPossess();
	}

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetVisibility(false, true);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComp->Stop();
		MeshComp->bPauseAnims = true;
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->StopMovementImmediately();
	}
}

void AEnemyCharacter::EnableEnemyAfterRespawn()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetVisibility(true, true);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		MeshComp->bPauseAnims = false;
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}
}

void AEnemyCharacter::RespawnEnemy()
{
	UE_LOG(LogTemp, Warning, TEXT("[EnemyRespawn] RespawnEnemy called: %s"), *GetName());

	SetActorLocationAndRotation(
		InitialSpawnLocation,
		InitialSpawnRotation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	EnableEnemyAfterRespawn();

	if (AIControllerClass)
	{
		SpawnDefaultController();
		AIController = Cast<AEnemyAIController>(GetController());
		BlackboardComp = AIController ? AIController->GetBlackboardComponent() : nullptr;

		UE_LOG(LogTemp, Warning, TEXT("[EnemyRespawn] Controller=%s Blackboard=%s"),
			*GetNameSafe(AIController), *GetNameSafe(BlackboardComp));

		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool(TEXT("bIsRangedEnemy"), IsRangedEnemy());
			BlackboardComp->SetValueAsFloat(TEXT("AttackRange"), GetPreferredAttackRange());
		}
	}

	if (AbilitySystemComponent && DefaultAttributeMetaDataTable && AttributeSetClassForInitStats)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		AbilitySystemComponent->InitStats(AttributeSetClassForInitStats, DefaultAttributeMetaDataTable);
		UE_LOG(LogTemp, Warning, TEXT("[EnemyRespawn] InitStats done"));
	}

	if (AbilitySystemComponent)
	{
		const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(TEXT("Enemy.State.Dead"));
		AbilitySystemComponent->RemoveLooseGameplayTag(DeadTag);
		UE_LOG(LogTemp, Warning, TEXT("[EnemyRespawn] Dead tag removed"));
	}

	bIsDead = false;

	UE_LOG(LogTemp, Warning, TEXT("[EnemyRespawn] Respawn finished"));
}

void AEnemyCharacter::HideDeadBody()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetVisibility(false, true);
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComp->bPauseAnims = true;
	}
}

float AEnemyCharacter::GetDeathMontageLength() const
{
	return DeathMontage ? DeathMontage->GetPlayLength() : 0.f;
}