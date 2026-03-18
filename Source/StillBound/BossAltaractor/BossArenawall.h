//BossArenawall.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossArenaWall.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class STILLBOUND_API ABossArenaWall : public AActor
{
	GENERATED_BODY()
	
public:	
	ABossArenaWall();

protected:

	virtual void BeginPlay() override;

	// 블로킹 콜리전
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall")
	TObjectPtr<UBoxComponent> WallCollision;

	//선택적 비주얼 메시 ,투명 장벽 머티리얼 적용 권장
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall")
	TObjectPtr<UStaticMeshComponent> WallMesh;

public:	

	//AltarActor
	UFUNCTION(BlueprintCallable, Category = "Wall")
	void SetWallSize(float Width, float Height);

	UPROPERTY(BlueprintReadOnly, Category = "Wall")
	FVector WallFullSize = FVector::ZeroVector;
};
