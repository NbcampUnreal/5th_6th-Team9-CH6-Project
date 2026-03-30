
#include "BossAI/BossAICharacter.h"
#include "BossAI/BosAIAttributeSet.h"
#include "BossAI/BossAIController.h"
#include "AIController.h"
#include "UI/USB_UIManager.h"
#include "UI/DamageNumberActor.h"
#include "Character/PlayerController_SB.h"
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

	APlayerController_SB* PC = Cast<APlayerController_SB>(GetWorld()->GetFirstPlayerController());

	if (PC && PC->UIManager)
	{
		PC->UIManager->ShowBossHP(FText::FromString(TEXT("Ancient Guardian")));
		PC->UIManager->UpdateBossHP(H, MH);
	}

	UE_LOG(LogTemp, Warning, TEXT("[Boss] After InitStats H=%.1f / %.1f"), H, MH);
}

void ABossAICharacter::ShowDamageNumber(float Damage)
{
	if (!DamageNumberClass) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	FVector CamLocation;
	FRotator CamRotation;
	PC->GetPlayerViewPoint(CamLocation, CamRotation);

	FVector Forward = CamRotation.Vector();
	FVector Right = FRotationMatrix(CamRotation).GetUnitAxis(EAxis::Y);
	FVector Up = FRotationMatrix(CamRotation).GetUnitAxis(EAxis::Z);

	FVector SpawnLocation = CamLocation + Forward * FMath::RandRange(350.f, 450.f);

	SpawnLocation += Right * FMath::RandRange(-70.f, 70.f);
	SpawnLocation += Up * FMath::RandRange(-40.f, 40.f);

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

void ABossAICharacter::HandleDeath()
{
	if (bIsDead) return;
	bIsDead = true;


	if (APlayerController_SB* PC = Cast<APlayerController_SB>(GetWorld()->GetFirstPlayerController()))
	{
		if (PC->UIManager)
		{
			PC->UIManager->ShowGameClear(true); 
		}
	}

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