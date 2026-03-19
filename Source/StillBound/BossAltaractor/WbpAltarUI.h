//WbpAltarUI.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WbpAltarUI.generated.h"

class AAltarActor;
class UTextBlock;
class UButton;
class UHorizontalBox;
class UImage;

UCLASS()
class STILLBOUND_API UWbpAltarUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Altar|UI")
	void InitWithAltar(AAltaractor* InAltar);

	// 슬롯 상태 갱신 (석판 삽입 후 호출) 
	UFUNCTION(BlueprintCallable, Category = "Altar|UI")
	void RefreshSlots();

protected:
	virtual void NativeConstruct() override;

	// =====바인딩, BP 디자이너에서 같은 이름으로 위젯 생성

	// 경고,상태 문구
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Warning;

	// 석판 넣기 버튼 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Insert;

	// 슬롯 이미지 배열
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Slot_0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Slot_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Slot_2;

	//채워진 슬롯 브러시 ,BP에서 석판 아이콘 설정
	UPROPERTY(EditDefaultsOnly, Category = "Altar|UI")
	FSlateBrush FilledSlotBrush;

	// 빈 슬롯 브러시 
	UPROPERTY(EditDefaultsOnly, Category = "Altar|UI")
	FSlateBrush EmptySlotBrush;

private:
	UPROPERTY()
	TObjectPtr<AAltaractor> LinkedAltar;

	UFUNCTION()
	void OnInsertButtonClicked();

	//슬롯 이미지 배열로 접근하기 위한 헬퍼
	UImage* GetSlotImage(int32 Index) const;
};
