#include "Items/CraftingStation.h"
#include "Character/PlayerCharacter_SB.h"
#include "Character/PlayerController_SB.h"

ACraftingStation::ACraftingStation()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	InteractableData.InteractableType = EInteractableType::Device;
	InteractableData.Name = FText::FromString(TEXT("Items"));
	InteractableData.Action = FText::FromString(TEXT("F to Craft"));
	InteractableData.Quantity = 0;
	InteractableData.InteractionDuration = 0.f;
}

void ACraftingStation::Interact_Implementation(APlayerCharacter_SB* PlayerCharacter)
{
	if (!PlayerCharacter) return;

	APlayerController_SB* PC = Cast<APlayerController_SB>(PlayerCharacter->GetController());
	if (!PC) return;

	if (PC->GetOverlayInputState() != EOverlayInputState::Gameplay)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CraftingStation] Not in Gameplay state"));
		return;
	}

	if (StationTag.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[CraftingStation] StationTag is None"));
		return;
	}

	if (!RecipeTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CraftingStation] RecipeTable is null"));
		return;
	}

	PlayerCharacter->OpenCraftingUI(StationTag, RecipeTable);
}

FInteractableData ACraftingStation::GetInteractableData_Implementation()
{
	return InteractableData;
}

void ACraftingStation::BeginFocus_Implementation()
{
	if (Mesh)
	{
		Mesh->SetRenderCustomDepth(true);
	}
}

void ACraftingStation::EndFocus_Implementation()
{
	if (Mesh)
	{
		Mesh->SetRenderCustomDepth(false);
	}
}

