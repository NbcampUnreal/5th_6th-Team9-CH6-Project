

#include "AI/EnemyCharacter.h"
#include "AI/EnemyAIController.h"
#include "EnemyVisualRow.h"
#include "Engine/DataTable.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/AIAttributeSet.h"
#include "AIController.h"
#include "UI/DamageNumberActor.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	AttributeSetClassForInitStats = UAIAttributeSet::StaticClass();

	AIControllerClass = AEnemyAIController::StaticClass();
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();


	ApplyVisualFromDataTable();

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

void AEnemyCharacter::HandleDeath()
{
	if (bIsDead) return;
	bIsDead = true;

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->UnPossess();
	}

	SetActorEnableCollision(false);
	SetLifeSpan(0.1f);
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
	SpawnLocation.Z += FMath::RandRange(120.f, 150.f);

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