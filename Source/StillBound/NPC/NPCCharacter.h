// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InteractableInterface.h"
#include "Interface/InteractionInterface.h"
#include "Character/PlayerCharacter_SB.h"
#include "DialogueComponent.h"
#include "NPCCharacter.generated.h"

class UDialogueWidget;

UENUM(BlueprintType)
enum class ENPCRegion : uint8
{
	Forest UMETA(DisplayName = "숲 (지역 A)"),
	//Snowfield UMETA(DisplayName = "설산 (지역 B)"),
	//Desert UMETA(DisplayName = "사막 (지역 C)"),
	//Volcano UMETA(DisplayName = "화산 (지역 D)")
};

UCLASS()
class STILLBOUND_API ANPCCharacter : public ACharacter, public IInteractableInterface, public IInteractionInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANPCCharacter();

	virtual void BeginFocus_Implementation() override;
	virtual void EndFocus_Implementation() override;
	virtual void BeginInteract_Implementation() override;
	virtual void EndInteract_Implementation() override;
	virtual void Interact_Implementation(AActor* InteractorActor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FInteractableData InstanceInteractableData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Dialogue")
	UDialogueComponent* DialogueComponent;

	virtual FInteractableData GetInteractableData_Implementation() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UDialogueWidget> DialogueWidgetClass;

	//위젯인스턴스
	UPROPERTY()
	UDialogueWidget* DialogueWidget;



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//NPC Info
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Info")
	FString NPCName = TEXT("주민");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Info")
	ENPCRegion Region = ENPCRegion::Forest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Info")
	FText NPCEdscription;

	//Interaction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Interaction")
	float InteractionDistance = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Interaction")
	FText InteractionPrompt = FText::FromString(TEXT("대화하기"));

	//Interaction Interface
	virtual bool StartInteraction_Implementation(AActor* Interactor) override;
	virtual void EndInteraction_Implementation(AActor* Interactor) override;
	virtual bool CanInteraction_Implementation(AActor* Interactor) const override;
	virtual float GetInteractionDistance_Implementation() const override;
	virtual FText GetInteractionText_Implementation(AActor* Interactor) const override;
	virtual bool IsInteracting_Implementation() const override;

	//블루프린트 커스텀
	//플레이어가 감지됐을 경우 (Alert)
	UFUNCTION(BlueprintImplementableEvent, Category = "NPC|Events")
	void OnPlayerDetected(AActor* Player);

	//플레이어를 잃었을 경우 (Idle)
	UFUNCTION(BlueprintImplementableEvent, Category = "NPC|Events")
	void OnPlayerLost();

	//상호작용 시작되었을 때
	UFUNCTION(BlueprintImplementableEvent, Category = "NPC|Events")
	void OnInteractionStarted(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, Category = "NPC|Events")
	void OnInteractionEnded(AActor* Interactor);

	//AI Controller 접근
	UFUNCTION(BlueprintPure, Category = "NPC")
	ANPCAIController* GetNPCAIController() const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "NPC")
	bool bIsInteracting = false;

	UPROPERTY(BlueprintReadOnly, Category = "NPC")
	AActor* CurrentInteractor = nullptr;

	UFUNCTION()
	void HandleDialogueStarted(const FDialogueRow& DialogueData);

	UFUNCTION()
	void HandleDialogueUpdated(const FDialogueRow& DialogueData);

	UFUNCTION()
	void HandleDialogueEnded();

	UFUNCTION()
	void HandleOptionSelected(int32 OptionIndex);

private:
	//상태변화 모니터링..??
	void MonitorStateChanges();
	uint8 LastNPCState = 0;

};
