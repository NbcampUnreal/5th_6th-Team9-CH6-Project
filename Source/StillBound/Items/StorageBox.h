#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractionInterface.h"
#include "StorageBox.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UInventoryComponent;

UCLASS()
class STILLBOUND_API AStorageBox : public AActor, public IInteractionInterface
{
	GENERATED_BODY()

public:
	AStorageBox();

	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	virtual void Interact_Implementation(APlayerCharacter_SB* PlayerCharacter) override;
	virtual FInteractableData GetInteractableData_Implementation() override;
	virtual void BeginFocus_Implementation() override;
	virtual void EndFocus_Implementation() override;


	void DropAllStoredItems();

	UFUNCTION(BlueprintCallable)
	UInventoryComponent* GetContainerInventory() const { return ContainerInventory; }

	UFUNCTION(BlueprintCallable)
	void BreakStorageBox();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInventoryComponent> ContainerInventory;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	FInteractableData InteractableData;
};

//인터랙션 인터페이스 상속받아서 하기 