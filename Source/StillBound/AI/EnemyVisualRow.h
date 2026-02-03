
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
};