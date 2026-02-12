
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "EnemyVisualRow.generated.h"


USTRUCT(BlueprintType)
struct STILLBOUND_API FEnemyVisualRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Id")
	int32 EnemyId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	TObjectPtr<USkeletalMesh> Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	TSubclassOf<UAnimInstance> AnimClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
	TObjectPtr<UAnimMontage> DeathMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
	TObjectPtr<UAnimMontage> GetHitMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Size", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float MeshScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Size", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float CapsuleRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Size", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float CapsuleHalfHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Size")
	FVector MeshRelativeLocation = FVector::ZeroVector;
};