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

	virtual void BeginPlay() override;

	FVector2D WorldToUV(const FVector& WorldLocation) const;

	FVector UVToWorld(const FVector2D& UV, float Z = 0.f) const;

protected:

	void AutoCalculateWorldBounds();

	UPROPERTY(EditAnywhere, Category = "Map")
	FVector2D WorldMin;

	UPROPERTY(EditAnywhere, Category = "Map")
	FVector2D WorldMax;

};
