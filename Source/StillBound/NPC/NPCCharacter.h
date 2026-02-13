// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/InteractionInterface.h"
#include "Character/PlayerCharacter_SB.h"
#include "DialogueComponent.h"
#include "NPCCharacter.generated.h"

class UDialogueWidget;
class ANPCAIController;

UENUM(BlueprintType)
enum class ENPCRegion : uint8
{
	Forest UMETA(DisplayName = "숲 (지역 A)"),
	//Snowfield UMETA(DisplayName = "설산 (지역 B)"),
	//Desert UMETA(DisplayName = "사막 (지역 C)"),
	//Volcano UMETA(DisplayName = "화산 (지역 D)")
};

UCLASS()
class STILLBOUND_API ANPCCharacter : public ACharacter,
    public IInteractionInterface
{
    GENERATED_BODY()

public:
    ANPCCharacter();

    // === 인터페이스 구현 ===
    virtual void BeginFocus_Implementation() override;
    virtual void EndFocus_Implementation() override;
    virtual void EndInteract_Implementation() override;
    virtual void Interact_Implementation(APlayerCharacter_SB* PlayerCharacter) override;
    virtual FInteractableData GetInteractableData_Implementation() override;
    virtual float GetInteractionDistance_Implementation() override;

    // === 대화 컴포넌트 ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    UDialogueComponent* DialogueComponent;

    // === Widget ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UDialogueWidget> DialogueWidgetClass;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === 내부 함수 ===
    void MonitorStateChanges();

    ANPCAIController* GetNPCAIController() const;

    // === 대화 이벤트 핸들러 ===
    UFUNCTION()
    void OnDialogueStart(const FDialogueRow& DialogueData);

    UFUNCTION()
    void OnDialogueUpdate(const FDialogueRow& DialogueData);

    UFUNCTION()
    void OnDialogueEnd();

    UFUNCTION()
    void OnOptionSelected(int32 OptionIndex);

    // === 블루프린트 이벤트 ===
    UFUNCTION(BlueprintImplementableEvent, Category = "NPC")
    void OnPlayerDetected(AActor* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC")
    void OnPlayerLost();

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC")
    void OnInteractionStarted(AActor* Interactor);

    UFUNCTION(BlueprintImplementableEvent, Category = "NPC")
    void OnInteractionEnded(AActor* Interactor);

    // === 상태 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName = TEXT("NPC");

    UPROPERTY(BlueprintReadOnly, Category = "NPC")
    bool bIsInteracting = false;

    UPROPERTY(BlueprintReadOnly, Category = "NPC")
    AActor* CurrentInteractor = nullptr;

private:
    uint8 LastNPCState = 0;

    // Widget 인스턴스
    UPROPERTY()
    UDialogueWidget* DialogueWidget = nullptr;
};
