

#include "AI/EnemyCharacter.h"
#include "AI/EnemyAIController.h"
#include "EnemyVisualRow.h"
#include "Engine/DataTable.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/AIAttributeSet.h"
#include "AIController.h"
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

	UE_LOG(LogTemp, Warning, TEXT("[EnemyCharacter] Applied DT EnemyId=%d (RowName=%s) Mesh=%s AnimClass=%s Montage=%s"),
		EnemyId,
		*FoundRowName.ToString(),
		*GetNameSafe(FoundRow->Mesh),
		*GetNameSafe(FoundRow->AnimClass.Get()),
		*GetNameSafe(FoundRow->AttackMontage));
}

void AEnemyCharacter::HandleDeath()
{
	UE_LOG(LogTemp, Error, TEXT("[Enemy] HandleDeath CALLED. Auth=%d Name=%s"),
		HasAuthority(), *GetName());

	if (bIsDead) return;
	bIsDead = true;

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
		AIC->UnPossess();
	}

	SetActorEnableCollision(false);

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
	}

	SetLifeSpan(3.0f);
}