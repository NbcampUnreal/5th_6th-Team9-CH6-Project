// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BaseAIController.generated.h"

UENUM(BlueprintType)
enum class EAIState : uint8
{
    Idle,
    Suspicious,
    Chasing
};
/**
 * 
 */
UCLASS()
class STILLBOUND_API ABaseAIController : public AAIController
{
	GENERATED_BODY()
	
public:
    // 생성자
    ABaseAIController();

    // 추적할 타겟(플레이어)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    AActor* TargetActor;

    // 추적 시작/중지 함수
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartChasing(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopChasing();

    // 거리 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float ChaseRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float LoseInterestRadius = 2000.0f;


    // 시야 감지 설정
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

    // 감지 이벤트 처리 함수
    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // 상태 변환 함수
    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateAIState();

    // 현재 상태 반환
    UFUNCTION(BlueprintPure, Category = "AI")
    EAIState GetCurrentState() const { return CurrentState; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    // 타겟 추적 중인지 여부
    bool bIsChasing = false;

    // 현재 AI 상태
    EAIState CurrentState = EAIState::Idle;

    // 마지막으로 타겟을 본 위치
    FVector LastKnownLocation;
};
