//WbpAltarUI.cpp

#include "BossAltaractor/WbpAltarUI.h"
#include "BossAltaractor/AltarActor.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"

void UWbpAltarUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Insert)
	{
		Button_Insert->OnClicked.AddDynamic(this, &UWbpAltarUI::OnInsertButtonClicked);
	}

	// 초기 상태: 경고 문구 숨김
	if (TextBlock_Warning)
	{
		TextBlock_Warning->SetVisibility(ESlateVisibility::Hidden);
	}
}

 void UWbpAltarUI::InitWithAltar(AAltaractor* InAltar)
{
	 LinkedAltar = InAltar;
	 RefreshSlots();
}

 void UWbpAltarUI::RefreshSlots()
 {
	 if (!LinkedAltar) return;

	 const int32 Filled = LinkedAltar->GetFilledSlotCount();
	 const int32 Required = LinkedAltar->GetRequiredSlotCount();

	 // 슬롯 이미지 업데이트
	 for (int32 i = 0; i < Required; ++i)
	 {
		 UImage* SlotImg = GetSlotImage(i);
		 if (!SlotImg) continue;

		 if (i < Filled)
		 {
			 SlotImg->SetBrush(FilledSlotBrush);
		 }
		 else
		 {
			 SlotImg->SetBrush(EmptySlotBrush);
		 }
	 }

	 // 경고 문구는 평소 숨김
	 if (TextBlock_Warning)
	 {
		 TextBlock_Warning->SetVisibility(ESlateVisibility::Hidden);
	 }
 }

void UWbpAltarUI::OnInsertButtonClicked()
{
	if (!LinkedAltar) return;

	const bool bSuccess = LinkedAltar->TryInsertTablet();

	if (!bSuccess)
	{
		// 석판 부족 경고
		if (TextBlock_Warning)
		{
			TextBlock_Warning->SetText(
				FText::FromString(TEXT("3 stone tablets are needed"))
			);
			TextBlock_Warning->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}

	// 성공 시 슬롯 갱신, 경고 숨김
	RefreshSlots();
}

UImage* UWbpAltarUI::GetSlotImage(int32 Index) const
{
	switch (Index)
	{
	case 0: return Slot_0;
	case 1: return Slot_1;
	case 2: return Slot_2;
	default: return nullptr;
	}
}
