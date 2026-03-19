// Copyright Epic Games, Inc. All Rights Reserved.

#include "StillBoundGameMode.h"

AStillBoundGameMode::AStillBoundGameMode()
{
	// 게임 시작 시 기본 부활 위치 초기화
	RespawnTransform = FTransform::Identity;
}

void AStillBoundGameMode::SetRespawnTransform(const FTransform& NewTransform)
{
	RespawnTransform = NewTransform;
	UE_LOG(LogTemp, Log, TEXT("[GameMode] 부활 위치 갱신: %s"), *RespawnTransform.ToString());
}

FTransform AStillBoundGameMode::GetRespawnTransform() const
{
	return RespawnTransform;
}