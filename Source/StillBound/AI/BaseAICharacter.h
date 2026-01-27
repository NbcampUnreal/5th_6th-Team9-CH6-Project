
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Character/BaseCharacter_SB.h"
#include "BaseAICharacter.generated.h"

UCLASS()
class STILLBOUND_API ABaseAICharacter : public ABaseCharacter_SB
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseAICharacter();

	void HandleDeath();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 타겟 위치로 이동
	UFUNCTION(BlueprintCallable, Category = "AI Movement")
	void MoveToTarget();

	// RVO 회피 활성화/비활성화
	UFUNCTION(BlueprintCallable, Category = "RVO")
	void SetRVOAvoidanceEnabled(bool bEnable);

public:
	// 이동할 타겟 액터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Movement")
	AActor* TargetActor;

	// RVO 회피 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO")
	float AvoidanceRadius = 300.0f;
	// RVO 계급 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO")
	float AvoidanceWeight = 0.5f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	bool bIsDead = false;

private:
	// AI 컨트롤러 캐싱
	class AAIController* AIController;
};
