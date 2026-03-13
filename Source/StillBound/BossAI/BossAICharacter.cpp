
#include "BossAI/BossAICharacter.h"
#include "BossAI/BosAIAttributeSet.h"
#include "BossAI/BossAIController.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


ABossAICharacter::ABossAICharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	BossAttributeSet = CreateDefaultSubobject<UBosAIAttributeSet>(TEXT("BossAttributeSet"));
	AttributeSetClassForInitStats = UBosAIAttributeSet::StaticClass();

	AIControllerClass = ABossAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ABossAICharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!AbilitySystemComponent) return;

	const float H = AbilitySystemComponent->GetNumericAttribute(UBosAIAttributeSet::GetHealthAttribute());
	const float MH = AbilitySystemComponent->GetNumericAttribute(UBosAIAttributeSet::GetMaxHealthAttribute());

	UE_LOG(LogTemp, Warning, TEXT("[Boss] After InitStats H=%.1f / %.1f"), H, MH);
}

void ABossAICharacter::HandleDeath()
{
	if (bIsDead) return;
	bIsDead = true;

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
		AIC->UnPossess();
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

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorEnableCollision(false);

	SetLifeSpan(0.1f);
}