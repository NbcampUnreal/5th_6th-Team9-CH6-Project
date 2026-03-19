// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AN_SendRangedFireEvent.generated.h"


UCLASS(meta = (DisplayName = "Ranged Fire Event"))
class STILLBOUND_API UAN_SendRangedFireEvent : public UAnimNotify
{
    GENERATED_BODY()

public:
    UAN_SendRangedFireEvent();

    virtual FString GetNotifyName_Implementation() const override;

    // //추가: 몽타주 특정 프레임에서 Event.Ranged.Fire 전송
    virtual void Notify(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference
    ) override;

protected:
    // //추가: 보낼 Gameplay Event 태그
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged")
    FGameplayTag EventTag;

    // //추가: ASC 없는 액터면 보내지 않게 안전장치
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Ranged")
    bool bRequireAbilitySystem = true;
};