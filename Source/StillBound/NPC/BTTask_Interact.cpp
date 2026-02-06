// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/BTTask_Interact.h"
#include "Interface/InteractionInterface.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

UBTTask_Interact::UBTTask_Interact()
{
    NodeName = "Interact";
    bNotifyTick = true;

    // Blackboard Key 필터 설정
    InteractionTargetKey.AddObjectFilter(
        this,
        GET_MEMBER_NAME_CHECKED(UBTTask_Interact, InteractionTargetKey),
        AActor::StaticClass()
    );
}

EBTNodeResult::Type UBTTask_Interact::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 초기화
    ElapsedTime = 0.0f;
    bInteractionStarted = false;
    CachedTargetActor = nullptr;
    CachedAIController = nullptr;
    CachedControlledPawn = nullptr;

    // AI Controller 가져오기
    CachedAIController = OwnerComp.GetAIOwner();
    if (!CachedAIController)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_Interact: No AI Controller"));
        return EBTNodeResult::Failed;
    }

    // Controlled Pawn 가져오기
    CachedControlledPawn = CachedAIController->GetPawn();
    if (!CachedControlledPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_Interact: No Controlled Pawn"));
        return EBTNodeResult::Failed;
    }

    // Blackboard에서 Target Actor 가져오기
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_Interact: No Blackboard Component"));
        return EBTNodeResult::Failed;
    }

    CachedTargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(InteractionTargetKey.SelectedKeyName));
    if (!CachedTargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_Interact: No Target Actor in Blackboard"));
        return EBTNodeResult::Failed;
    }

    // IInteractionInterface 구현 확인
    if (!CachedTargetActor->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
    {
        UE_LOG(LogTemp, Error, TEXT("BTTask_Interact: Target does not implement IInteractionInterface"));
        return EBTNodeResult::Failed;
    }

    // 거리 체크
    if (bCheckDistance)
    {
        float Distance = FVector::Dist(
            CachedControlledPawn->GetActorLocation(),
            CachedTargetActor->GetActorLocation()
        );

        float MaxDistance = IInteractionInterface::Execute_GetInteractionDistance(CachedTargetActor);
        if (MaxDistance <= 0.0f)
        {
            MaxDistance = 200.0f;  // 기본값
        }

        if (Distance > MaxDistance)
        {
            UE_LOG(LogTemp, Warning, TEXT("BTTask_Interact: Target too far (%.1f > %.1f)"),
                Distance, MaxDistance);
            return EBTNodeResult::Failed;
        }
    }

    // 타겟 바라보기
    if (bLookAtTarget)
    {
        CachedAIController->SetFocus(CachedTargetActor, EAIFocusPriority::Gameplay);
    }

    // 상호작용 시작
    IInteractionInterface::Execute_Interact(CachedTargetActor, nullptr);

    bInteractionStarted = true;

    UE_LOG(LogTemp, Log, TEXT("BTTask_Interact: Interaction started with %s"),
        *CachedTargetActor->GetName());

    // 지속 시간이 0이면 즉시 종료 (아이템 줍기, 스위치 등)
    if (InteractionDuration <= 0.0f)
    {
        if (bAutoEndInteraction)
        {
            IInteractionInterface::Execute_EndInteract(CachedTargetActor);
        }

        CleanupInteraction();
        return EBTNodeResult::Succeeded;
    }

    // Duration이 있으면 TickTask에서 처리 (대화는 여기서 수동 종료)
    return EBTNodeResult::InProgress;
}

void UBTTask_Interact::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    // 유효성 검사
    if (!ValidateInteraction(OwnerComp))
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_Interact: Validation failed during tick"));
        CleanupInteraction();
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 타겟 바라보기 (부드러운 회전)
    if (bLookAtTarget && CachedControlledPawn && CachedTargetActor)
    {
        FVector Direction = CachedTargetActor->GetActorLocation() - CachedControlledPawn->GetActorLocation();
        Direction.Z = 0.0f;  // 수평 회전만

        FRotator TargetRotation = Direction.Rotation();
        FRotator CurrentRotation = CachedControlledPawn->GetActorRotation();

        FRotator NewRotation = FMath::RInterpTo(
            CurrentRotation,
            FRotator(CurrentRotation.Pitch, TargetRotation.Yaw, CurrentRotation.Roll),
            DeltaSeconds,
            5.0f  // 회전 속도
        );

        CachedControlledPawn->SetActorRotation(NewRotation);
    }

    // 경과 시간 업데이트
    ElapsedTime += DeltaSeconds;

    // 지속 시간 체크
    if (ElapsedTime >= InteractionDuration)
    {
        // 자동 종료
        if (bAutoEndInteraction && CachedTargetActor)
        {
            IInteractionInterface::Execute_EndInteract(CachedTargetActor);
            UE_LOG(LogTemp, Log, TEXT("BTTask_Interact: Interaction ended (auto) after %.1fs"), ElapsedTime);
        }

        CleanupInteraction();
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

bool UBTTask_Interact::ValidateInteraction(UBehaviorTreeComponent& OwnerComp)
{
    // Task가 시작되지 않았으면 실패
    if (!bInteractionStarted)
    {
        return false;
    }

    // 캐싱된 참조들이 유효한지 확인
    if (!IsValid(CachedAIController) || !IsValid(CachedControlledPawn) || !IsValid(CachedTargetActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_Interact: Cached references became invalid"));
        return false;
    }

    // Target이 여전히 IInteractionInterface를 구현하는지 확인
    if (!CachedTargetActor->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_Interact: Target no longer implements IInteractionInterface"));
        return false;
    }

    return true;
}

void UBTTask_Interact::CleanupInteraction()
{
    // Focus 해제
    if (bLookAtTarget && IsValid(CachedAIController))
    {
        CachedAIController->ClearFocus(EAIFocusPriority::Gameplay);
    }

    // 캐싱된 참조 초기화
    CachedTargetActor = nullptr;
    CachedAIController = nullptr;
    CachedControlledPawn = nullptr;

    bInteractionStarted = false;
    ElapsedTime = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("BTTask_Interact: Cleanup completed"));
}