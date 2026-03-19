#include "Items/Pickup.h"
#include "Items/ItemBase.h"
#include "Character/PlayerCharacter_SB.h"
#include "Character/PlayerController_SB.h"
#include "Inventory/InventoryComponent.h"
#include "Components/SphereComponent.h"
//#include "Public/Data/ItemData.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetSimulatePhysics(true);
	SetRootComponent(PickupMesh);

	AutoPickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AutoPickupSphere"));
	AutoPickupSphere->SetupAttachment(PickupMesh);
	AutoPickupSphere->InitSphereRadius(150.f);
	AutoPickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AutoPickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AutoPickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	if (AutoPickupSphere)
	{
		AutoPickupSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnAutoPickupSphereBeginOverlap);
	}

	InitializePickup(UItemBase::StaticClass(), ItemQuantity);

}

FInteractableData APickup::GetInteractableData_Implementation()
{
	return InstanceInteractableData;
}

void APickup::InitializePickup(const TSubclassOf<UItemBase> BaseClass, const int32 InQuantity)
{
	if (!ItemRowHandle.IsNull())
	{
		const FItemDataRow* ItemData = ItemRowHandle.GetRow<FItemDataRow>(ItemRowHandle.RowName.ToString());

		ItemReference = NewObject<UItemBase>(this, BaseClass);

		ItemReference->ID = ItemData->ID;
		ItemReference->ItemType = ItemData->ItemType;
		ItemReference->ItemQuality = ItemData->ItemQuality;
		ItemReference->NumericData = ItemData->NumericData;
		ItemReference->TextData = ItemData->TextData;
		ItemReference->AssetData = ItemData->AssetData;

		ItemReference->ItemStatistics = ItemData->ItemStatistics;
		//ItemReference->PickupActorClass = ItemData->PickupActorClass;
		ItemReference->EquipWeaponClass = ItemData->EquipWeaponClass;

		ItemReference->ConsumableEffectClass = ItemData->ConsumableEffectClass;         
		ItemReference->ConsumableSetByCallerTag = ItemData->ConsumableSetByCallerTag;

		ItemReference->NumericData.bIsStackable = ItemData->NumericData.MaxStackSize > 1;
		InQuantity <= 0 ? ItemReference->SetQuantity(1) : ItemReference->SetQuantity(InQuantity);

		PickupMesh->SetStaticMesh(ItemData->AssetData.Mesh);

		UpdateInteractableData();
	}
}

void APickup::InitializeDrop(UItemBase* ItemToDrop, const int32 InQuantity)
{
	ItemReference = ItemToDrop;
	InQuantity <= 0 ? ItemReference->SetQuantity(1) : ItemReference->SetQuantity(InQuantity);
	ItemReference->NumericData.Weight = ItemToDrop->GetItemSingleWeight();
	PickupMesh->SetStaticMesh(ItemToDrop->AssetData.Mesh);

	UpdateInteractableData();
}

void APickup::UpdateInteractableData()
{
	InstanceInteractableData.InteractableType = EInteractableType::Pickup;
	InstanceInteractableData.Action = ItemReference->TextData.InteractionText;
	InstanceInteractableData.Name = ItemReference->TextData.Name;
	InstanceInteractableData.Quantity = ItemReference->Quantity;
	//InteractableData = InstanceInteractableData;
}

void APickup::BeginFocus_Implementation()
{
	if (PickupMesh)
	{
		PickupMesh->SetRenderCustomDepth(true);
	}
}

void APickup::EndFocus_Implementation()
{
	if (PickupMesh)
	{
		PickupMesh->SetRenderCustomDepth(false);
	}
}

void APickup::Interact_Implementation(APlayerCharacter_SB* PlayerCharacter)
{
	if (PlayerCharacter)
	{
		TakePickup(PlayerCharacter);
	}
}


void APickup::TakePickup(const APlayerCharacter_SB* Taker)
{
	if (!IsPendingKillPending())
	{
		if (ItemReference)
		{
			if (UInventoryComponent* PlayerInventory = Taker->GetInventory())
			{
				const FItemAddResult AddResult = PlayerInventory->HandleAddItem_AutoHotbarFirst(ItemReference);

				switch (AddResult.OperationResult)
				{
				case EItemAddResult::IAR_NoItemAdded:
					break;
				case EItemAddResult::IAR_PartialAmountItemAdded:
				{
					UpdateInteractableData();
					Taker->UpdateInteractionWidget();

					if (APlayerController_SB* PC = Cast<APlayerController_SB>(Taker->GetController()))
					{
						if (PC->UIManager)
						{
							PC->UIManager->ShowPickupText(
								FText::Format(
									FText::FromString(TEXT("{0} +{1}")),
									ItemReference->TextData.Name,
									ItemReference->Quantity
								)
							);
						}
					}

					break;
				}
				
				case EItemAddResult::IAR_AllItemAdded:
				{
					if (APlayerController_SB* PC = Cast<APlayerController_SB>(Taker->GetController()))
					{
						if (PC->UIManager)
						{
							PC->UIManager->ShowPickupText(
								FText::Format(
									FText::FromString(TEXT("{0} +{1}")),
									ItemReference->TextData.Name,
									ItemReference->Quantity
								)
							);
						}
					}

					Destroy();
					break;
				}
				}

				UE_LOG(LogTemp, Warning, TEXT("%s"), *AddResult.ResultMessage.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Player inventory component is invalid"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Pickup internal ItemReference is invalid"));
		}
	}
}

#if WITH_EDITOR
void APickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName ChangedPropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(FDataTableRowHandle, RowName))
	{
		if (!ItemRowHandle.IsNull())
		{
			const FItemDataRow* ItemData = ItemRowHandle.GetRow<FItemDataRow>(ItemRowHandle.RowName.ToString());
			PickupMesh->SetStaticMesh(ItemData->AssetData.Mesh);
		}
	}
}
#endif 

bool APickup::IsAutoPickupGold() const
{
	if (!ItemReference)
	{
		return false;
	}

	return ItemReference->ID == FName(TEXT("700001"));
}

void APickup::OnAutoPickupSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!IsAutoPickupGold())
	{
		return;
	}

	APlayerCharacter_SB* PlayerCharacter = Cast<APlayerCharacter_SB>(OtherActor);
	if (!PlayerCharacter)
	{
		return;
	}

	TakePickup(PlayerCharacter);
}
