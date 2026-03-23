#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BuildingData.generated.h"

UENUM(BlueprintType)
enum class EBuildCategory : uint8
{
    Production UMETA(DisplayName = "Production"),
    Structure UMETA(DisplayName = "Structure")
};

UENUM(BlueprintType)
enum class EBuildType : uint8
{
    Foundation UMETA(DisplayName = "Foundation"),
    Wall       UMETA(DisplayName = "Wall"),
    Roof       UMETA(DisplayName = "Roof"),
    Door       UMETA(DisplayName = "Door"),
    Workbench  UMETA(DisplayName = "Workbench"),
    Furnace    UMETA(DisplayName = "Furnace")
};

UENUM(BlueprintType)
enum class EBuildSnapRule : uint8
{
    None,                  // 자유배치
    GroundOnly,            // 바닥만
    FoundationEdgeOnly,    // 토대 모서리, 벽만
    OnTopOfWall            // 벽 위
};

USTRUCT(BlueprintType)
struct FBuildCost
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName ItemID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Count = 1;
};

USTRUCT(BlueprintType)
struct FBuildingDataRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName BuildingID = NAME_None;

    /// 나중에 소분류카테고리 확장을 위한 속성 지금당장은 무시해도됨
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName SubCategory = NAME_None;
    /// --------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTexture2D* Icon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EBuildCategory Category = EBuildCategory::Structure;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EBuildType BuildType = EBuildType::Foundation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EBuildSnapRule SnapRule = EBuildSnapRule::GroundOnly;

    // 실제 설치될 액터
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<AActor> BuildActorClass;

    // 고스트 프리뷰용 액터
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<AActor> GhostActorClass;

    // UI용 대표 메시(선택사항)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UStaticMesh* PreviewMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FBuildCost> Costs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
    FRotator PreviewRotationOffset = FRotator::ZeroRotator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
    FVector PreviewLocationOffset = FVector::ZeroVector;
};
