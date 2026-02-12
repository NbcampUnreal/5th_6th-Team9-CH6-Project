
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	int32 EnemyId = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0.0"))
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0"))
	int32 MaxAliveFromThisEntry = 0;
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
	UPROPERTY(VisibleAnywhere, Category = "Spawn")
	TObjectPtr<UBoxComponent> SpawnBox;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AEnemyCharacter> EnemyClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<FEnemySpawnEntry> SpawnEntries;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0"))
	int32 MaxAlive = 6;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.0"))
	float ActivationDistance = 999999.0f;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.0"))
	float MinDistanceFromPlayer = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.1"))
	float SpawnCheckInterval = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.0"))
	float RespawnDelay = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "1"))
	int32 MaxFindLocationTries = 12;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.0"))
	float SpawnCollisionRadius = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.0"))
	float BoxEdgePadding = 80.0f;

	FTimerHandle SpawnTimerHandle;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> AliveActors;

	TMap<int32, int32> AliveCountByEnemyId;

	double LastVacancyTime = -1.0;

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
