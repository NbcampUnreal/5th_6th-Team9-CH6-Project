
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AISpawnVolume.generated.h"

class UBoxComponent;
class UNavigationSystemV1;
class AEnemyCharacter;

USTRUCT(BlueprintType)
struct STILLBOUND_API FEnemySpawnEntry
{
	GENERATED_BODY()

	// EnemyCharacter가 DataTable에서 찾는 EnemyId
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	int32 EnemyId = 1;

	// 가중치(확률 비율)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0.0"))
	float Weight = 1.0f;

	// 이 엔트리로 스폰할 수 있는 최대 수(선택)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0"))
	int32 MaxAliveFromThisEntry = 0; // 0이면 제한 없음
};

UCLASS()
class STILLBOUND_API AAISpawnVolume : public AActor
{
	GENERATED_BODY()

public:
	AAISpawnVolume();

protected:
	virtual void BeginPlay() override;

private:
	// ===== Components =====
	UPROPERTY(VisibleAnywhere, Category = "Spawn")
	TObjectPtr<UBoxComponent> SpawnBox;

	// ===== Spawn Settings =====
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AEnemyCharacter> EnemyClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<FEnemySpawnEntry> SpawnEntries;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0"))
	int32 MaxAlive = 6;

	// 플레이어가 이 거리 안에 들어오면 스폰 활성화
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.0"))
	float ActivationDistance = 999999.0f;

	// 플레이어와 너무 가까운 곳에 스폰되는 것 방지
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.0"))
	float MinDistanceFromPlayer = 0.0f;

	// 스폰 체크 주기(틱 대신 타이머)
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.1"))
	float SpawnCheckInterval = 1.5f;

	// 죽고 난 뒤 재스폰 대기(여기선 “스폰 시도”에서 Alive가 비면 알아서 채움)
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.0"))
	float RespawnDelay = 0.0f;

	// 네비메시 랜덤 포인트를 찾을 때 시도 횟수
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "1"))
	int32 MaxFindLocationTries = 12;

	// 스폰 위치 주변 겹침 방지용 반경
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.0"))
	float SpawnCollisionRadius = 10.0f;

	// SpawnBox 안에서 랜덤 점 뽑을 때, 가장자리 패딩
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.0"))
	float BoxEdgePadding = 80.0f;

	// ===== Runtime =====
	FTimerHandle SpawnTimerHandle;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> AliveActors;

	// 엔트리별 현재 Alive 카운트(EnemyId 기준)
	TMap<int32, int32> AliveCountByEnemyId;

	// 마지막으로 죽어서 비게 된 시간(간단한 리스폰 딜레이용)
	double LastVacancyTime = -1.0;

	// ===== Helpers =====
	void SpawnTick();

	APawn* GetPlayerPawn() const;
	bool IsPlayerInRange(APawn* PlayerPawn) const;

	bool ShouldSpawnNow() const;
	int32 GetAliveCount() const;
	void CleanupAliveList();

	bool FindSpawnLocation(APawn* PlayerPawn, FVector& OutLocation, FRotator& OutRotation) const;

	int32 PickEnemyIdByWeight() const;
	bool CanSpawnEnemyId(int32 EnemyId) const;

	AEnemyCharacter* SpawnEnemyDeferred(int32 EnemyId, const FVector& Location, const FRotator& Rotation);

	UFUNCTION()
	void HandleSpawnedActorDestroyed(AActor* DestroyedActor);
};
