// Copyright Epic Games, Inc. All Rights Reserved.

#include "StillBoundGameMode.h"

AStillBoundGameMode::AStillBoundGameMode()
{
	// 게임 시작 시 기본 부활 위치 초기화
	RespawnTransform = FTransform::Identity;
}

void AStillBoundGameMode::SetRespawnTransform(const FTransform& NewTransform)
{
	// 새 체크포인트 혹은 저장된 위치로 부활위치 등록
	RespawnTransform = NewTransform;
	//UE_LOG(LogTemp, Log, TEXT("[GameMode] 부활 위치 갱신: %s"), *RespawnTransform.ToString());
	//유효한 부활 위치 표시
	bHasRespawnTransform = true;
}

bool AStillBoundGameMode::HasRespawnTransform() const
{
	return bHasRespawnTransform;
}

FTransform AStillBoundGameMode::GetRespawnTransform() const
{
	return RespawnTransform;
}