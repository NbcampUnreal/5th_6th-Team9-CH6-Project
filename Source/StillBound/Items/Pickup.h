// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractionInterface.h"
#include "Components/SphereComponent.h"
#include "Pickup.generated.h"

class UItemBase;
class UDataTable;

UCLASS()
class STILLBOUND_API APickup : public AActor, public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	///===============================================================================
	/// PROPERTIES & VARIABLES
	///===============================================================================


	///===============================================================================
	/// FUNCTIONS
	///===============================================================================
	APickup();

	void InitializePickup(const TSubclassOf<UItemBase> BaseClass, const int32 InQuantity);

	void InitializeDrop(UItemBase* ItemToDrop, const int32 InQuantity);

	FORCEINLINE UItemBase* GetItemData() { return ItemReference; }

	virtual void BeginFocus_Implementation() override;
	virtual void EndFocus_Implementation() override;

	virtual FInteractableData GetInteractableData_Implementation() override;

protected:
	///===============================================================================
	/// PROPERTIES & VARIABLES
	///===============================================================================
	UPROPERTY(VisibleAnywhere, Category = "Pickup|Components")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UPROPERTY(VisibleAnywhere, Category = "Pickup|Components")
	TObjectPtr<USphereComponent> AutoPickupSphere;

	UFUNCTION()
	void OnAutoPickupSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	bool IsAutoPickupGold() const;

	UPROPERTY(VisibleAnywhere, Category = "Pickup|Item Reference")
	TObjectPtr<UItemBase> ItemReference;

	//UPROPERTY(EditInstanceOnly, Category = "Pickup|Item Initialization")
	//UDataTable* ItemDataTable;

	//UPROPERTY(EditInstanceOnly, Category = "Pickup|Item Initialization")
	//FName DesiredItemID;

	UPROPERTY(EditInstanceOnly, Category = "Pickup|Item Initialization")
	int32 ItemQuantity;

	UPROPERTY(VisibleAnywhere, Category = "Pickup|Interaction")
	FInteractableData InstanceInteractableData;

	UPROPERTY(EditInstanceOnly, Category = "Pickup|Item Initialization")
	FDataTableRowHandle ItemRowHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Drop")
	bool bUseAutoDestroyForDroppedItem = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Drop", meta = (ClampMin = "0.0"))
	float DroppedItemLifeSeconds = 300.f;

	void ApplyDroppedItemLifeSpan();

	///===============================================================================
	/// FUNCTIONS
	///===============================================================================

	virtual void BeginPlay() override;

	//virtual void Interact(APlayerCharacter_SB* PlayerCharacter) override;
	virtual void Interact_Implementation(class APlayerCharacter_SB* PlayerCharacter) override;
	void UpdateInteractableData();

	void TakePickup(const APlayerCharacter_SB* Taker);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
