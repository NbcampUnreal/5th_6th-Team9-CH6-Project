#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemData.generated.h"


class AWeaponBase;

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Weapon UMETA(DisplayName = "Weapon"),
	Armor UMETA(DisplayName = "Armor"),
	Tool UMETA(DisplayName = "Tool"),
	Ammo UMETA(DisplayName = "Ammo"),
	Material UMETA(DisplayName = "Material"),
	Building UMETA(DisplayName = "Building"),
	Consumable UMETA(DisplayName = "Consumable")
};

UENUM(BlueprintType)
enum class EItemQuality : uint8
{
	Common UMETA(DisplayName = "Common"),
	Rare UMETA(DisplayName = "Rare"),
	Unique UMETA(DisplayName = "Unique"),
	Legendary UMETA(DisplayName = "Legendary")
};

UENUM(BlueprintType)
enum class EWeaponKind : uint8
{
	Melee UMETA(DisplayName = "Melee"),
	Ranged UMETA(DisplayName = "Ranged")
};

USTRUCT(BlueprintType)
struct FItemStatistics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float ArmorRating;

	UPROPERTY(EditAnywhere)
	float DamageValue;

	UPROPERTY(EditAnywhere)
	float RestorationAmount;

	UPROPERTY(EditAnywhere)
	float SellValue;

	UPROPERTY(EditAnywhere)
	float BuyValue;
};

USTRUCT(BlueprintType)
struct FItemTextData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FText Name;

	UPROPERTY(EditAnywhere)
	FText Description;

	UPROPERTY(EditAnywhere)
	FText InteractionText;

	UPROPERTY(EditAnywhere)
	FText UsageText;
};

USTRUCT()
struct FItemNumericData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float Weight;

	UPROPERTY(EditAnywhere)
	int32 MaxStackSize;

	UPROPERTY(EditAnywhere)
	bool bIsStackable;
};

USTRUCT()
struct FItemAssetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere)
	UStaticMesh* Mesh;
};


#pragma region Master ItemDataTable

USTRUCT(BlueprintType)
struct FItemDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Item")
	FName ID;

	UPROPERTY(EditAnywhere, Category = "Item")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, Category = "Item")
	EItemQuality ItemQuality;

	UPROPERTY(EditAnywhere, Category = "Item")
	FItemStatistics ItemStatistics;

	UPROPERTY(EditAnywhere, Category = "Item")
	FItemTextData TextData;

	UPROPERTY(EditAnywhere, Category = "Item")
	FItemNumericData NumericData;

	UPROPERTY(EditAnywhere, Category = "Item")
	FItemAssetData AssetData;

	UPROPERTY(EditAnywhere, Category = "Item")
	TSoftClassPtr<AActor> PickupActorClass;

	UPROPERTY(EditAnywhere, Category = "Item|Equip")
	TSoftClassPtr<AWeaponBase> EquipWeaponClass;
};

#pragma endregion

#pragma region Weapon Stat Table

USTRUCT(BlueprintType)
struct FWeaponStatRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponKind WeaponKind = EWeaponKind::Melee;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AttackDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AttackRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 MaxDurability = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 MaxAmmo = 0;
};

#pragma endregion

#pragma region Armor Stat Table

USTRUCT(BlueprintType)
struct FArmorStatRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor")
	float Defense = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor")
	float MaxHP = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor")
	int32 MaxDurability = 0;

};
#pragma endregion

#pragma region Tool Stat Table

USTRUCT(BlueprintType)
struct FToolStatRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tool")
	int32 Tier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tool")
	float AttackDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tool")
	int32 MaxDurability = 0;

};

#pragma endregion

#pragma region Ammo Stat Table

USTRUCT(BlueprintType)
struct FAmmoStatRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	float Gravity = 0;

};

#pragma endregion

#pragma region Consumable Item Stat Table

USTRUCT(BlueprintType)
struct FConsumableItemStatRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ConsumableItem")
	float AddHealth = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ConsumableItem")
	float AddMana = 0;

};

#pragma endregion

#pragma region Building Table

USTRUCT(BlueprintType)
struct FBuildingRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	TSubclassOf<AActor> BuildActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	TSubclassOf<AActor> GhostActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	int32 MaxDurability = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	float Gravity = 0;

};

#pragma endregion

#pragma region Recipe Result Table

USTRUCT(BlueprintType)
struct FRecipeResultRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	int32 ResultItemID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	int32 ResultCount = 1;

};
#pragma endregion

#pragma region Recipe Material Table

USTRUCT(BlueprintType)
struct FRecipeMaterialRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	int32 RecipeID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	int32 MaterialItemID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	int32 Count = 0;

};
#pragma endregion