#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapWorldManager.generated.h"

UCLASS()
class STILLBOUND_API AMapWorldManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AMapWorldManager();

	UPROPERTY(EditAnywhere, Category = "Map")
	FVector2D WorldMin;

	UPROPERTY(EditAnywhere, Category = "Map")
	FVector2D WorldMax;

	FVector2D WorldToUV(const FVector& WorldLocation) const;
};
