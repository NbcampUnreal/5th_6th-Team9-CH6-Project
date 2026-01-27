// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BaseAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

ABaseAIController::ABaseAIController()
{
    // 매 프레임 틱 활성화
    PrimaryActorTick.bCanEverTick = true;

    // 시야 감지 설정 생성
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = DetectionRadius;
    SightConfig->LoseSightRadius = LoseInterestRadius;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;


}

void ABaseAIController::BeginPlay()
{
    Super::BeginPlay();

    // 인지 컴포넌트 초기화 후 컴포넌트 세팅
    if (SightConfig && GetPerceptionComponent())
    {
        GetPerceptionComponent()->ConfigureSense(*SightConfig);
        GetPerceptionComponent()->SetDominantSense(SightConfig->GetSenseImplementation());
        GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseAIController::OnPerceptionUpdated);
    }

    // 기본 타겟으로 플레이어 설정 (선택적)
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (PlayerCharacter)
    {
        TargetActor = PlayerCharacter;
    }
}

void ABaseAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 상태 업데이트
    UpdateAIState();

    // 추적 로직
    if (bIsChasing && TargetActor)
    {
        APawn* ControlledPawn = GetPawn();
        if (ControlledPawn)
        {
            float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());

            // 추적 거리 내에 있을 때만 추적
            if (Distance <= ChaseRadius)
            {
                // 플레이어를 향해 이동
                MoveToActor(TargetActor, 100.0f);

                // 마지막 위치 갱신
                LastKnownLocation = TargetActor->GetActorLocation();

                // 디버그 시각화 (시뮬레이션 중 확인용)
#if WITH_EDITOR
                DrawDebugLine(
                    GetWorld(),
                    ControlledPawn->GetActorLocation(),
                    TargetActor->GetActorLocation(),
                    FColor::Red,
                    false,
                    -1.0f,
                    0,
                    2.0f
                );
#endif
            }
            else if (Distance > LoseInterestRadius)
            {
                // 추적 중지
                StopChasing();
            }
        }
    }
}

void ABaseAIController::StartChasing(AActor* Target)
{
    TargetActor = Target;
    bIsChasing = true;

    if (Target)
    {
        // 마지막 위치 업데이트
        LastKnownLocation = Target->GetActorLocation();
    }

    // 상태 변경
    CurrentState = EAIState::Chasing;
}

void ABaseAIController::StopChasing()
{
    bIsChasing = false;
    StopMovement();

    // 상태 변경
    CurrentState = EAIState::Idle;
}

void ABaseAIController::UpdateAIState()
{
    if (!TargetActor) return;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    float DistanceToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());

    switch (CurrentState)
    {
    case EAIState::Idle:
        if (DistanceToTarget <= DetectionRadius)
        {
            CurrentState = EAIState::Suspicious;
        }
        break;

    case EAIState::Suspicious:
        if (DistanceToTarget <= ChaseRadius)
        {
            StartChasing(TargetActor);
        }
        else if (DistanceToTarget > DetectionRadius)
        {
            CurrentState = EAIState::Idle;
        }
        break;

    case EAIState::Chasing:
        if (DistanceToTarget > LoseInterestRadius)
        {
            StopChasing();
        }
        break;
    }
}

void ABaseAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    // 플레이어 캐릭터인지 확인
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    if (Actor == PlayerCharacter)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            // 플레이어 감지 성공
            TargetActor = Actor;

            // 거리에 따라 상태 변경
            APawn* ControlledPawn = GetPawn();
            if (ControlledPawn)
            {
                float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), Actor->GetActorLocation());

                if (Distance <= ChaseRadius)
                {
                    StartChasing(Actor);
                }
                else if (Distance <= DetectionRadius)
                {
                    CurrentState = EAIState::Suspicious;
                }
            }
        }
        else
        {
            // 플레이어 감지 실패 (시야에서 사라짐)
            if (CurrentState == EAIState::Chasing)
            {
                // 마지막으로 본 위치로 이동
                MoveToLocation(LastKnownLocation, 50.0f);

                // 의심 상태로 전환
                CurrentState = EAIState::Suspicious;
            }
        }
    }
}