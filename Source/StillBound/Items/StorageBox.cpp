#include "Items/StorageBox.h"

AStorageBox::AStorageBox()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AStorageBox::BeginPlay()
{
	Super::BeginPlay();
	
}

void AStorageBox::Destroyed()
{
}

void AStorageBox::BreakStorageBox()
{
}

void AStorageBox::DropAllStoredItems()
{
}


