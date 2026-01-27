

#include "AI/BaseAICharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/AIAttributeSet.h"
#include "AIController.h"


// Sets default values
ABaseAICharacter::ABaseAICharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//  InitStats는 AI용 AttributeSet으로
	AttributeSetClassForInitStats = UAIAttributeSet::StaticClass();

	// RVO 회피 시스템 활성화
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->bUseRVOAvoidance = true;
		MovementComponent->AvoidanceConsiderationRadius = AvoidanceRadius;
		MovementComponent->AvoidanceWeight = 0.5f;
	}
}

void ABaseAICharacter::HandleDeath()
{

	UE_LOG(LogTemp, Error, TEXT("[AI] HandleDeath CALLED. Auth=%d  Name=%s"),
		HasAuthority(), *GetName());

	if (bIsDead) return;
	bIsDead = true;

	// 이동/AI 중지
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
		AIC->UnPossess();
	}

	// 충돌 끄기
	SetActorEnableCollision(false);

	// 이동 멈추기
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
	}

	SetLifeSpan(3.0f);

}

// Called when the game starts or when spawned
void ABaseAICharacter::BeginPlay()
{
	Super::BeginPlay();

	UAIAttributeSet* AS = AbilitySystemComponent
		? const_cast<UAIAttributeSet*>(AbilitySystemComponent->GetSet<UAIAttributeSet>())
		: nullptr;

	UE_LOG(LogTemp, Warning, TEXT("[AI] ASC Set<UAIAttributeSet>=%p"), AS);

	if (AS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AI] After InitStats H=%.1f / %.1f"),
			AS->GetHealth(), AS->GetMaxHealth());
	}

	// AI 컨트롤러 참조 얻기
	AIController = Cast<AAIController>(GetController());

	// AI 컨트롤러가 없으면 로그 출력
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s is not controlled by an AIController. Movement functions will not work."), *GetName());
	}
	else if (TargetActor)
	{
		// 타겟 액터가 설정되어 있으면 자동으로 이동 시작
		MoveToTarget();
	}
}

// Called every frame
void ABaseAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABaseAICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABaseAICharacter::MoveToTarget()
{
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToTarget failed: No AI Controller for %s"), *GetName());
		return;
	}

	if (!TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToTarget failed: No Target Actor set for %s"), *GetName());
		return;
	}

	// 타겟 액터를 향해 이동
	AIController->MoveToActor(
		TargetActor,    // 목표 액터
		50.0f,          // 도착 판정 반경
		true,           // 충돌 영역이 겹치면 도착으로 간주
		true,           // 경로 탐색 사용
		false           // 목적지를 네비게이션 메시에 투영(Projection)하지 않음
	);

	UE_LOG(LogTemp, Display, TEXT("%s moving to target: %s"),
		*GetName(), *TargetActor->GetName());
}

void ABaseAICharacter::SetRVOAvoidanceEnabled(bool bEnable)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->bUseRVOAvoidance = bEnable;
	}
}