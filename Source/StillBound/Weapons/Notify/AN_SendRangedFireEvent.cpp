// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Notify/AN_SendRangedFireEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"
#include "GameFramework/Actor.h"

UAN_SendRangedFireEvent::UAN_SendRangedFireEvent()
{
    // //추가: 현재 Ranged GA가 기다리는 기본 태그와 동일하게 설정
    EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Ranged.Fire"), false);
    bRequireAbilitySystem = true;
}

FString UAN_SendRangedFireEvent::GetNotifyName_Implementation() const
{
    return TEXT("RangedFireEvent");
}

void UAN_SendRangedFireEvent::Notify(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference
)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp)
    {
        return;
    }

    AActor* OwnerActor = MeshComp->GetOwner();
    if (!OwnerActor)
    {
        return;
    }

    if (!EventTag.IsValid())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[AN_SendRangedFireEvent] EventTag is invalid. Owner=%s"),
            *GetNameSafe(OwnerActor));
        return;
    }

    if (bRequireAbilitySystem)
    {
        UAbilitySystemComponent* ASC =
            UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);

        if (!ASC)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[AN_SendRangedFireEvent] ASC not found. Owner=%s"),
                *GetNameSafe(OwnerActor));
            return;
        }
    }

    FGameplayEventData Payload;
    Payload.EventTag = EventTag;
    Payload.Instigator = OwnerActor;
    Payload.Target = OwnerActor;

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        OwnerActor,
        EventTag,
        Payload
    );

#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Log,
        TEXT("[AN_SendRangedFireEvent] Sent Event=%s Owner=%s"),
        *EventTag.ToString(),
        *GetNameSafe(OwnerActor));
#endif
}