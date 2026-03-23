#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StorageBox.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UInventoryComponent;

UCLASS()
class STILLBOUND_API AStorageBox : public AActor
{
	GENERATED_BODY()

public:
	AStorageBox();

	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION(BlueprintCallable)
	UInventoryComponent* GetContainerInventory() const { return ContainerInventory; }

	UFUNCTION(BlueprintCallable)
	void BreakStorageBox();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> BoxMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> InteractionCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryComponent> ContainerInventory;

	void DropAllStoredItems();
};

//인터랙션 인터페이스 상속받아서 하기 