// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StillBoundGameMode.generated.h"

/**
 * Simple GameMode for a third person game
 */
UCLASS(abstract)
class AStillBoundGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	/** Constructor */
	AStillBoundGameMode();

	// 부활 위치 저장
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	FTransform RespawnTransform;

	// 부활 위치 존재여부 bool값
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Respawn")
	bool bHasRespawnTransform = false;

	// 부활 위치 갱신
	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void SetRespawnTransform(const FTransform& NewTransform);

	// 저장된 부활 위치를 가져옴
	UFUNCTION(BlueprintPure, Category = "Respawn")
	FTransform GetRespawnTransform() const;

	//이미 저장된 위치를 덮어쓰지 않게 하기
	UFUNCTION(BlueprintPure, Category = "Respawn")
	bool HasRespawnTransform() const;
};