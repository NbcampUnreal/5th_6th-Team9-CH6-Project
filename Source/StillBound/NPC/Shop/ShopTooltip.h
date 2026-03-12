// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopTooltip.generated.h"

class UTextBlock;
struct FItemDataRow;

/**
 * 
 */
UCLASS()
class STILLBOUND_API UShopTooltip : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // 아이템 정보 + 가격 설정
    void SetItemInfo(const FItemDataRow* ItemData, int32 Price);

protected:
    // 바인딩 위젯들
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ItemName;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ItemType;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ItemDescription;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ItemPrice;  // ← 추가!

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ItemStats;
};
