#include "Character/PlayerCharacter_SB.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Character/PlayerAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Interface/InteractionInterface.h"
#include "NPC/DialogueComponent.h"
#include "DrawDebugHelpers.h"
#include "UI/USB_UIManager.h"
#include "Components/SlateWrapperTypes.h"
#include "Character/PlayerController_SB.h"
#include "Items/Pickup.h"
#include "Weapons/WeaponBase.h"
#include "Subsystem/SBWorldSaveManagerSubsystem.h"
#include "Items/ItemBase.h"
#include "UI/UW_UIHUD.h"
#include "Build/BuildComponent.h"


APlayerCharacter_SB::APlayerCharacter_SB()
{
	PrimaryActorTick.bCanEverTick = true;

	//  �÷��̾� AttributeSet ����
	PlayerAttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("PlayerAttributeSet"));

	//  InitStats�� �� Ŭ������
	AttributeSetClassForInitStats = UPlayerAttributeSet::StaticClass();

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 97.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>("FollowCamera");
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	PlayerInventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("PlayerInventory"));
	PlayerInventory->SetSlotsCapacity(48);
	PlayerInventory->SetWeightCapacity(50.f);

	InteractionCheckFrequency = 0.1f;
	InteractionCheckDistance = 225.f;

	BuildComponent = CreateDefaultSubobject<UBuildComponent>(TEXT("BuildComponent"));
}

FInteractableData APlayerCharacter_SB::GetInteractableData_Implementation()
{
	return FInteractableData();
}

void APlayerCharacter_SB::BeginPlay()
{
	Super::BeginPlay();

	if (auto* Sub = GetGameInstance() ? GetGameInstance()->GetSubsystem<USBWorldSaveManagerSubsystem>() : nullptr)
	{
		const bool bOk = Sub->LoadCurrentWorldAttributesToPawn(this);
		UE_LOG(LogTemp, Warning, TEXT("[Gameplay] LoadCurrentWorldAttributesToPawn -> %d"), bOk);
	}

	if (!AbilitySystemComponent) return;

	const UPlayerAttributeSet* AS = AbilitySystemComponent->GetSet<UPlayerAttributeSet>();

	UE_LOG(LogTemp, Warning, TEXT("[Player] ASC Set<UPlayerAttributeSet>=%p"), AS);

	const float H = AbilitySystemComponent->GetNumericAttribute(UPlayerAttributeSet::GetHealthAttribute());
	const float MH = AbilitySystemComponent->GetNumericAttribute(UPlayerAttributeSet::GetMaxHealthAttribute());

	UE_LOG(LogTemp, Warning, TEXT("[Player] After InitStats H=%.1f / %.1f"), H, MH);

	BuildComponent->Camera = FollowCamera;
}

bool APlayerCharacter_SB::EquipWeaponFromItem(UItemBase* Item)
{
	if (!Item || Item->ItemType != EItemType::Weapon) return false;
	if (!AbilitySystemComponent) { UE_LOG(LogTemp, Error, TEXT("[Equip] ASC is NULL")); return false; }

	// DT에서 지정한 무기 BP
	if (Item->EquipWeaponClass.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Equip] EquipWeaponClass is NULL. ItemID=%s"), *Item->ID.ToString());
		return false;
	}

	TSubclassOf<AWeaponBase> WeaponClass = Item->EquipWeaponClass.LoadSynchronous();
	if (!WeaponClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Equip] EquipWeaponClass load failed. ItemID=%s"), *Item->ID.ToString());
		return false;
	}

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp) return false;

	// 교체 장착
	if (EquippedWeapon)
	{
		UnequipWeapon(true);
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass, Params);
	if (!NewWeapon) return false;

	NewWeapon->AttachToComponent(
		MeshComp,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		StartingWeaponSocketName
	);

	// ✅ DT 스탯(데미지)을 무기에 주입 (5번에서 추가할 함수)
	NewWeapon->InitFromItem(Item);

	// ✅ GA/GE 부여 (Spec.SourceObject=this 유지)
	NewWeapon->Equip(this, AbilitySystemComponent);

	EquippedWeapon = NewWeapon;

	UE_LOG(LogTemp, Log, TEXT("[Equip] Equipped %s (ItemID=%s, Damage=%.2f)"),
		*GetNameSafe(NewWeapon), *Item->ID.ToString(), NewWeapon->GetWeaponDamage());

	return true;
}

void APlayerCharacter_SB::UnequipWeapon(bool bDestroyWeaponActor)
{
	if (!EquippedWeapon) return;

	UE_LOG(LogTemp, Warning, TEXT("[Unequip] Weapon=%s Destroy=%d"),
		*GetNameSafe(EquippedWeapon), (int32)bDestroyWeaponActor);

	// 1) ASC에서 GA/GE 회수 + WeaponTypeTag 제거 (WeaponBase.cpp에 이미 구현됨)
	EquippedWeapon->Unequip();

	// 2) 손 소켓에서 분리
	EquippedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// (선택) 충돌/표시 처리 필요하면 여기서
	// EquippedWeapon->SetActorEnableCollision(false);
	// EquippedWeapon->SetActorHiddenInGame(true);

	// 3) 액터를 유지할지(재사용/인벤토리) 파괴할지 결정
	if (bDestroyWeaponActor)
	{
		EquippedWeapon->Destroy();
	}

	EquippedWeapon = nullptr;
}


void APlayerCharacter_SB::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
	{
		PerformInteractionCheck();
	}
	if (InteractionData.bIsInteracting && InteractionData.CurrentInteractable)
	{
		float Dist = FVector::Dist(GetActorLocation(), InteractionData.CurrentInteractable->GetActorLocation());

		if (Dist > 300.0f)
		{
			EndInteract(); 
		}
	}

	//채집 중 이동 감지 추가
	if (bIsGathering && InteractionData.bIsInteracting)
	{
		float MovedDist = FVector::Dist(GetActorLocation(), GatherStartLocation);
		if (MovedDist > 10.0f)//10cm이상 이동하면 취소
		{
			EndInteract();
			NotifyGatherEnd();
		}
	}
}

void APlayerCharacter_SB::PerformInteractionCheck()
{
	if (InteractionData.bIsInteracting) return;

	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	FVector TraceStart{ GetPawnViewLocation() };
	FVector TraceEnd{ TraceStart + (GetViewRotation().Vector() * InteractionCheckDistance) };

	float LookDirection = FVector::DotProduct(GetActorForwardVector(), GetViewRotation().Vector());

	if (LookDirection > 0)
	{
		/*DrawDebugLine(
			GetWorld(),
			TraceStart,
			TraceEnd,
			FColor::Red,
			false,
			1.f,
			2.f);*/

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		FHitResult TraceHit;

		if (GetWorld()->LineTraceSingleByChannel(
			TraceHit,
			TraceStart,
			TraceEnd,
			ECC_Visibility,
			QueryParams))
		{
			AActor* HitActor = TraceHit.GetActor();

			if (HitActor && HitActor != this)
			{
				if (HitActor->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
				{
					if (HitActor != InteractionData.CurrentInteractable)
					{
						FoundInteractable(HitActor);
					}

					return;
				}
			}
		}
	}

	NoInteractableFound();
}

void APlayerCharacter_SB::FoundInteractable(AActor* NewInteractable)
{
	if (IsInteracting())
	{
		EndInteract();
	}

	if (InteractionData.CurrentInteractable)
	{
		TargetInteractable = InteractionData.CurrentInteractable;
	
		IInteractionInterface::Execute_EndFocus(TargetInteractable.GetObject());
	}

	InteractionData.CurrentInteractable = NewInteractable;
	TargetInteractable = NewInteractable;

	auto* PC = Cast<APlayerController_SB>(GetController());
	if (!PC) return;

	USB_UIManager* UI = PC->UIManager;
	if(!UI) return;


	if (TargetInteractable.GetObject())
	{
		FInteractableData Data = IInteractionInterface::Execute_GetInteractableData(TargetInteractable.GetObject());
		UI->UpdateInteractionWidget(Data);
		IInteractionInterface::Execute_BeginFocus(TargetInteractable.GetObject());

	}

}

void APlayerCharacter_SB::NoInteractableFound()
{
	if (InteractionData.bIsInteracting && !InteractionData.CurrentInteractable)
	{
		EndInteract();
		return;
	}

	if (InteractionData.CurrentInteractable)
	{
		if (IsValid(TargetInteractable.GetObject()))
		{
			IInteractionInterface::Execute_EndFocus(TargetInteractable.GetObject());
		}

		auto* PC = Cast<APlayerController_SB>(GetController());
		if (!PC) return;

		USB_UIManager* UI = PC->UIManager;
		if (!UI) return;

		UI->HideInteractionWidget();

		InteractionData.CurrentInteractable = nullptr;
		TargetInteractable = nullptr;
	}
}

void APlayerCharacter_SB::BeginInteract()
{
	//verify nothing has changed with the interactble state since beginning interaction
	PerformInteractionCheck();

	if (!InteractionData.CurrentInteractable || !IsValid(TargetInteractable.GetObject()))
	{
		if (auto* PC = Cast<APlayerController_SB>(GetController()))
		{
			if (PC->UIManager)
			{
				PC->UIManager->HideInteractionWidget();
			}
		}
		return;
	}

	InteractionData.bIsInteracting = true;

	if (InteractionData.CurrentInteractable)
	{
		if (IsValid(TargetInteractable.GetObject()))
		{
			IInteractionInterface::Execute_BeginInteract(TargetInteractable.GetObject());
			FInteractableData TargetData = IInteractionInterface::Execute_GetInteractableData(TargetInteractable.GetObject());
			if (FMath::IsNearlyZero(TargetData.InteractionDuration, 0.1f))
			{
				Interact();
			}
			else
			{
				GetWorldTimerManager().SetTimer(
					TimerHandle_Interaction,
					this,
					&APlayerCharacter_SB::Interact,
					TargetData.InteractionDuration, 
					false);
			}
		}
	}
}

void APlayerCharacter_SB::EndInteract()
{
	if (InteractionData.CurrentInteractable)
	{
		IInteractionInterface::Execute_EndInteract(InteractionData.CurrentInteractable);
	}

	InteractionData.bIsInteracting = false;
	GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);

}

void APlayerCharacter_SB::Interact()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interaction);

	UObject* TargetObject = TargetInteractable.GetObject();
	if (!TargetObject)
	{
		UE_LOG(LogTemp, Error, TEXT("Interact: TargetInteractable.GetObject() is nullptr"));
		return;
	}

	AActor* TargetActor = Cast<AActor>(TargetObject);
	if (!IsValid(TargetActor))
	{
		UE_LOG(LogTemp, Error, TEXT("Interact: TargetActor is invalid"));
		return;
	}

	if (!TargetActor->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("Interact: Target does not implement IInteractionInterface"));
		return;
	}

	UDialogueComponent* DialogueComp = TargetActor->FindComponentByClass<UDialogueComponent>();
	if (DialogueComp)
	{
		if (auto* PC = Cast<APlayerController_SB>(GetController()))
		{
			if (PC->UIManager)
			{
				PC->UIManager->HideInteractionWidget();
			}
		}

		DialogueComp->OnDialogueEnded.RemoveAll(this);
		DialogueComp->OnDialogueEnded.AddDynamic(this, &APlayerCharacter_SB::EndInteract);
	
		InteractionData.bIsInteracting = true;
	}

	IInteractionInterface::Execute_Interact(TargetActor, this);

	if (DialogueComp == nullptr)
	{
		EndInteract();
	}
}

void APlayerCharacter_SB::SelectHotbarIndex(int32 NewIndex)
{
	if (!PlayerInventory) return;

	const int32 HotbarSize = PlayerInventory->GetHotbarCapacity();
	if (HotbarSize <= 0) return;

	NewIndex = (NewIndex % HotbarSize + HotbarSize) % HotbarSize;
	
	const bool bChanged = (CurrentHotbarIndex != NewIndex);
	CurrentHotbarIndex = NewIndex;

	if (auto* PC = Cast<APlayerController_SB>(GetController()))
	{
		if (PC->UIManager && PC->UIManager->GetHUD())
		{
			PC->UIManager->GetHUD()->SetSelectedHotbarIndex(CurrentHotbarIndex);
		}
	}

	HandleHotbarSelectionChanged();
}

void APlayerCharacter_SB::HandleHotbarSelectionChanged()
{
	SelectedConsumable = nullptr;

	if (!PlayerInventory) return;
	
	UItemBase* Item = PlayerInventory->GetItemInContainer(ESlotContainer::Hotbar, CurrentHotbarIndex);

	if(!Item)
	{
		UnequipWeapon();
		return;
	}

	if (Item->ItemType != EItemType::Weapon)
	{
		UnequipWeapon();
	}

	switch (Item->ItemType)
	{
	case EItemType::Weapon:
		//무기장착코드작성
		EquipWeaponFromItem(Item);
		break;

	case EItemType::Tool:
		//도구장착코드작성
		//EquipToolFromItem(Item);
		break;

	case EItemType::Armor:
	case EItemType::Ammo:
	case EItemType::Consumable:
		SelectedConsumable = Item;
		break;

	case EItemType::Material:
	case EItemType::Building:
	default:
		break;
	}
}

void APlayerCharacter_SB::UseSelectedHotbarItem()
{
	if (!PlayerInventory) return;

	if (!SelectedConsumable) return;
	if (SelectedConsumable->ItemType != EItemType::Consumable) return;

	UItemBase* Cur = PlayerInventory->GetItemInContainer(ESlotContainer::Hotbar, CurrentHotbarIndex);
	if (!Cur || Cur != SelectedConsumable) return;


	// 아이템 효과적용코드작성부분
	//ApplyConsumableEffectByID(Cur->ID);


	PlayerInventory->RemoveAmountInContainer(ESlotContainer::Hotbar, CurrentHotbarIndex, 1);

	Cur = PlayerInventory->GetItemInContainer(ESlotContainer::Hotbar, CurrentHotbarIndex);
	if (!Cur)
	{
		SelectedConsumable = nullptr;
	}
	else
	{
		SelectedConsumable = Cur;
	}
}

void APlayerCharacter_SB::OpenCraftingUI(FName InStationTag, UDataTable* InRecipeTable)
{
	APlayerController_SB* PlayerController = Cast<APlayerController_SB>(GetController());
	if (!PlayerController || !PlayerController->UIManager) return;

	UInventoryComponent* Inv = GetInventory();
	if (!Inv || !InRecipeTable) return;

	Inv->CurrentStationTag = InStationTag;
	Inv->RecipeDataTable = InRecipeTable;

	PlayerController->UIManager->OpenCraftingMenu(Inv);
}


void APlayerCharacter_SB::UpdateInteractionWidget() const
{
	UObject* InteractableObject = TargetInteractable.GetObject();

	if (IsValid(TargetInteractable.GetObject()))
	{
		auto* PC = Cast<APlayerController_SB>(GetController());
		if (!PC) return;

		USB_UIManager* UI = PC->UIManager;
		if (!UI) return;

		FInteractableData Data = IInteractionInterface::Execute_GetInteractableData(InteractableObject);
		UI->UpdateInteractionWidget(Data);
	}
}

void APlayerCharacter_SB::DropItemFromSlot(ESlotContainer FromContainer, int32 FromIndex, int32 QuantityToDrop)
{
	if (!PlayerInventory) return;

	UItemBase* ItemToDrop = PlayerInventory->GetItemInContainer(FromContainer, FromIndex);
	if (!ItemToDrop)
	{
		UE_LOG(LogTemp, Warning, TEXT("DropItemFromSlot: No item at %d / %d"), (int32)FromContainer, FromIndex);
		return;
	}

	if (QuantityToDrop <= 0)
	{
		QuantityToDrop = ItemToDrop->Quantity;
	}

	UItemBase* DropTemplate = ItemToDrop;
	if (QuantityToDrop < ItemToDrop->Quantity)
	{
		DropTemplate = ItemToDrop->CreateItemCopy();
		DropTemplate->ResetItemFlags();
		DropTemplate->SetQuantity(QuantityToDrop);
	}

	const int32 RemovedQuantity = PlayerInventory->RemoveAmountInContainer(FromContainer, FromIndex, QuantityToDrop);
	if (RemovedQuantity <= 0) return;
	if (!PickupClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	const FVector SpawnLocation{ GetActorLocation() + (GetActorForwardVector() * 50.f) };
	const FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

	APickup* Pickup = GetWorld()->SpawnActor<APickup>(PickupClass, SpawnTransform, SpawnParams);
	if (Pickup)
	{
		Pickup->InitializeDrop(DropTemplate, RemovedQuantity);
	}
}

void APlayerCharacter_SB::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->DisableMovement();
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
		{
			Anim->StopAllMontages(0.1f);
		}
	}

	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage, 1.5f);
		return;
	}
}

//============채집 기능 추가
void APlayerCharacter_SB::NotifyGatherStart(float Duration)
{
	bIsGathering = true;
	GatherStartLocation = GetActorLocation();

	if (APlayerController_SB* PC = Cast<APlayerController_SB>(GetController()))
	{
		PC->StartGatherProgress(Duration);
	}
}

void APlayerCharacter_SB::NotifyGatherEnd()
{
	bIsGathering = false;

	if (APlayerController_SB* PC = Cast<APlayerController_SB>(GetController()))
	{
		PC->EndGatherProgress();
	}
}

void APlayerCharacter_SB::DestroyActorComponent(UActorComponent* ComponentToDestroy)
{
	ComponentToDestroy->DestroyComponent();
}
