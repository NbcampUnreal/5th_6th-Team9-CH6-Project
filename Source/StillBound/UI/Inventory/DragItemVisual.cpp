#include "UI/Inventory/DragItemVisual.h"

void UDragItemVisual::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::HitTestInvisible);
}
