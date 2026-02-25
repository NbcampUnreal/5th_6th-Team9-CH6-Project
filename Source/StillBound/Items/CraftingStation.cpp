#include "Items/CraftingStation.h"
#include "Character/PlayerCharacter_SB.h"

ACraftingStation::ACraftingStation()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

}

void ACraftingStation::Interact_Implementation(APlayerCharacter_SB* PlayerCharacter)
{
	APlayerCharacter_SB* PC = Cast<APlayerCharacter_SB>(PlayerCharacter);
	if (!PC) return;

	PC->OpenCraftingUI(StationTag, RecipeTable);
}



