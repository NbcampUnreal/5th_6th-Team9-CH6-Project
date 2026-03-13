#include "UI/Map/MapWorldManager.h"
#include "Landscape.h"
#include "EngineUtils.h"

AMapWorldManager::AMapWorldManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AMapWorldManager::BeginPlay()
{
    Super::BeginPlay();
    AutoCalculateWorldBounds();
}

void AMapWorldManager::AutoCalculateWorldBounds()
{
    FBox WorldBounds(EForceInit::ForceInit);

    for (TActorIterator<ALandscape> It(GetWorld()); It; ++It)
    {
        WorldBounds += It->GetComponentsBoundingBox(true);
    }

    if (!WorldBounds.IsValid)
    {
        UE_LOG(LogTemp, Error, TEXT("[Map] Invalid WorldBounds"));
        return;
    }

    WorldMin = FVector2D(WorldBounds.Min.Y, WorldBounds.Min.X);
    WorldMax = FVector2D(WorldBounds.Max.Y, WorldBounds.Max.X);

    UE_LOG(LogTemp, Warning, TEXT("[Map] Auto Min: %s"), *WorldBounds.Min.ToString());
    UE_LOG(LogTemp, Warning, TEXT("[Map] Auto Max: %s"), *WorldBounds.Max.ToString());
}

FVector2D AMapWorldManager::WorldToUV(const FVector& WorldLocation) const
{
    FVector2D Pos2D(WorldLocation.Y, WorldLocation.X);

    FVector2D Size = WorldMax - WorldMin;

    if (Size.X <= KINDA_SMALL_NUMBER || Size.Y <= KINDA_SMALL_NUMBER)
    {
        return FVector2D(0.5f, 0.5f);
    }

    FVector2D UV = (Pos2D - WorldMin) / Size;

    UV.X = FMath::Clamp(UV.X, 0.f, 1.f);
    UV.Y = FMath::Clamp(UV.Y, 0.f, 1.f);

    return UV;
}

FVector AMapWorldManager::UVToWorld(const FVector2D& UV, float Z) const
{
    FVector2D Size = WorldMax - WorldMin;

    FVector2D World2D = WorldMin + UV * Size;

    return FVector(World2D.Y, World2D.X, Z);
}



