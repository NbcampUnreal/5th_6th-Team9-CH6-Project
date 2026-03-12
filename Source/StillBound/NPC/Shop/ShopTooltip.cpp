// ShopTooltip.cpp
// Fill out your copyright notice in the Description page of Project Settings.

#include "NPC/Shop/ShopTooltip.h"
#include "Data/ItemData.h"
#include "Components/TextBlock.h"

void UShopTooltip::SetItemInfo(const FItemDataRow* ItemData, int32 Price)
{
    if (!ItemData) return;

    // 아이템 이름
    if (TXT_ItemName)
    {
        TXT_ItemName->SetText(ItemData->TextData.Name);
    }

    // 아이템 타입
    if (TXT_ItemType)
    {
        FString TypeString;
        switch (ItemData->ItemType)
        {
        case EItemType::Weapon:
            TypeString = TEXT("Weapon");
            break;
        case EItemType::Tool:
            TypeString = TEXT("Tool");
            break;
        case EItemType::Consumable:
            TypeString = TEXT("Consumable");
            break;
        case EItemType::Material:
            TypeString = TEXT("Material");
            break;
        case EItemType::Ammo:
            TypeString = TEXT("Ammo");
            break;
        case EItemType::Armor:
            TypeString = TEXT("Armor");
            break;
        case EItemType::Building:
            TypeString = TEXT("Building");
            break;
        default:
            TypeString = TEXT("Other");
            break;
        }
        TXT_ItemType->SetText(FText::FromString(TypeString));
    }

    // 설명
    if (TXT_ItemDescription)
    {
        TXT_ItemDescription->SetText(ItemData->TextData.Description);
    }

    // 가격
    if (TXT_ItemPrice)
    {
        FString PriceText = FString::Printf(TEXT("Price: %d Gold"), Price);
        TXT_ItemPrice->SetText(FText::FromString(PriceText));
    }

    // 스탯 정보
    if (TXT_ItemStats)
    {
        FString StatsString;

        if (ItemData->ItemType == EItemType::Weapon ||
            ItemData->ItemType == EItemType::Tool)
        {
            StatsString += FString::Printf(TEXT("Damage: %.0f\n"),
                ItemData->ItemStatistics.DamageValue);
        }

        if (ItemData->ItemType == EItemType::Consumable)
        {
            StatsString += FString::Printf(TEXT("Restore: %.0f\n"),
                ItemData->ItemStatistics.RestorationAmount);
        }

        StatsString += FString::Printf(TEXT("Weight: %.1f"),
            ItemData->NumericData.Weight);

        TXT_ItemStats->SetText(FText::FromString(StatsString));
    }
}