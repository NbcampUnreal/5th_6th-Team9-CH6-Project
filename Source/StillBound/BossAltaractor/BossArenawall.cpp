#include "BossAltaractor/BossArenaWall.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

ABossArenaWall::ABossArenaWall()
{
	PrimaryActorTick.bCanEverTick = false;

	// 콜리전 루트
	WallCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("WallCollision"));
	RootComponent = WallCollision;

	WallCollision->SetCollisionProfileName(TEXT("BlockAll"));
	WallCollision->SetBoxExtent(FVector(50.f, 50.f, 250.f)); // SetWallSize에서 재설정됨

	// 비주얼 메시 (BP에서 투명 장벽 머티리얼 설정 권장)
	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	WallMesh->SetupAttachment(RootComponent);
	WallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 콜리전은 BoxComponent가 처리
}

void ABossArenaWall::BeginPlay()
{
	Super::BeginPlay();
}

void ABossArenaWall::SetWallSize(float Width, float Height)
{
    const FVector HalfExtent(100.f, Width * 0.5f, Height * 0.5f);
    WallCollision->SetBoxExtent(HalfExtent, true);

    // BP Construction Script에서 사용할 전체 크기 저장
    WallFullSize = HalfExtent * 2.f;

    //if (WallMesh)
    //{
    //    const float MeshBase = 100.f;
    //    WallMesh->SetRelativeScale3D(FVector(
    //        (HalfExtent.X * 2.f) / MeshBase,   // X 두께
    //        (HalfExtent.Y * 2.f) / MeshBase,   // Y 길이
    //        (HalfExtent.Z * 2.f) / MeshBase    // Z 높이
    //    ));

    //    // 메시 위치를 콜리전 중심에 맞춤
    //    WallMesh->SetRelativeLocation(FVector::ZeroVector);
    //}
}