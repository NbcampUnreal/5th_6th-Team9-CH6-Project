// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Notify/ANS_Melee.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectTypes.h" // FGameplayEventData
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"

static FGameplayTag TAG_HitboxOn()
{
    return FGameplayTag::RequestGameplayTag(TEXT("Event.Melee.Hitbox.On"), false);
}

static FGameplayTag TAG_HitboxOff()
{
    return FGameplayTag::RequestGameplayTag(TEXT("Event.Melee.Hitbox.Off"), false);
}

void UANS_Melee::NotifyBegin(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    float TotalDuration,
    const FAnimNotifyEventReference& EventReference
)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (!MeshComp) return;
    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    const FGameplayTag Tag = TAG_HitboxOn();
    if (!Tag.IsValid()) return;

    FGameplayEventData Data;
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, Tag, Data);
}

void UANS_Melee::NotifyEnd(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference
)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (!MeshComp) return;
    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    const FGameplayTag Tag = TAG_HitboxOff();
    if (!Tag.IsValid()) return;

    FGameplayEventData Data;
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, Tag, Data);
}