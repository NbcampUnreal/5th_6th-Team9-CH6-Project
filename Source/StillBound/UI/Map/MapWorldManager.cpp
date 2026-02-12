#include "UI/Map/MapWorldManager.h"

AMapWorldManager::AMapWorldManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

FVector2D AMapWorldManager::WorldToUV(const FVector& WorldLocation) const
{
    FVector2D Pos2D(WorldLocation.Y, WorldLocation.X);

    FVector2D Size = WorldMax - WorldMin;

    // 안전 장치
    if (Size.X <= KINDA_SMALL_NUMBER || Size.Y <= KINDA_SMALL_NUMBER)
    {
        return FVector2D(0.5f, 0.5f);
    }

    FVector2D UV = (Pos2D - WorldMin) / Size;

    
    UV.X = FMath::Clamp(UV.X, 0.f, 1.f);
    UV.Y = FMath::Clamp(UV.Y, 0.f, 1.f);

    return UV;
}

