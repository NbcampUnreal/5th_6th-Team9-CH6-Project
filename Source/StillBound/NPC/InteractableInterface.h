// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 상호작용 가능한 모든 객체(NPC, 오브젝트 등)
 */
class STILLBOUND_API IInteractableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	//상호작용의 시작, 상호작용을 시도하는 액터(플레이어), return 상호작용 성공 시작여부
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool StartInteraction(AActor* Interactor);


	//상호작용 종료, 상호작용을 종료하는 액터
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void EndInteraction(AActor* Interactor);


	//상호작용 가능 여부, 상호작용을 시도하는 액터, return 상호작용 가능 여부
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteraction(AActor* Interactor) const;


	//상호작용 가능 거리
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	float GetInteractionDistance() const;


	//상호작용 UI
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionText(AActor* Interactor) const;


	//현재 상호작용 중인지, return 상태
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool IsInteracting() const;

};
