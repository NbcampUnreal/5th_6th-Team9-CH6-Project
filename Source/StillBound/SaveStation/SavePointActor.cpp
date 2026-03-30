// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveStation/SavePointActor.h"
#include "Kismet/GameplayStatics.h"
#include "StillBoundGameMode.h"
#include "Character/PlayerCharacter_SB.h"

// Sets default values
ASavePointActor::ASavePointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);


	InteractableData.InteractableType = EInteractableType::Device;
	InteractableData.InteractionDuration = 0.1f; // 버튼을 누르고 있어야 하는 시간

}

// Called when the game starts or when spawned
void ASavePointActor::BeginPlay()
{
	Super::BeginPlay();
	
}

FInteractableData ASavePointActor::GetInteractableData_Implementation()
{
	return InteractableData;
}

void ASavePointActor::BeginFocus_Implementation()
{
	// 바라볼 때 외곽선(Custom Depth) 켜기
	if (Mesh) Mesh->SetRenderCustomDepth(true);
}

void ASavePointActor::EndFocus_Implementation()
{
	// 시선을 돌리면 외곽선 끄기
	if (Mesh) Mesh->SetRenderCustomDepth(false);
}

void ASavePointActor::BeginInteract_Implementation() {}
void ASavePointActor::EndInteract_Implementation() {}

void ASavePointActor::Interact_Implementation(APlayerCharacter_SB* PlayerCharacter)
{
	// 1. 현재 맵의 게임 모드를 가져와서 우리가 만든 AStillBoundGameMode로 캐스팅합니다.
	if (AStillBoundGameMode* GM = Cast<AStillBoundGameMode>(UGameplayStatics::GetGameMode(this))) 
	{
		if (PlayerCharacter)
		{
			FTransform SaveTransform = PlayerCharacter->GetActorTransform();

			// 3. 게임 모드에 부활 위치를 덮어씌웁니다!
			GM->SetRespawnTransform(SaveTransform);
			// 디버그 메시지
			UE_LOG(LogTemp, Warning, TEXT("[SavePoint] 부활 위치가 갱신되었습니다."));

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("위치가 저장되었습니다"));
			}
		}
	}
}