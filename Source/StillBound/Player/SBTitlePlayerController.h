#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SBTitlePlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

UCLASS()
class STILLBOUND_API ASBTitlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Title|UI")
	TSubclassOf<UUserWidget> TitleRootWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Title|Input")
	TObjectPtr<UInputMappingContext> IMC_System;

	UPROPERTY(EditDefaultsOnly, Category = "Title|Input")
	TObjectPtr<UInputMappingContext> IMC_Movement;

	UPROPERTY(EditDefaultsOnly, Category = "Title|Input")
	TObjectPtr<UInputMappingContext> IMC_Abilities;

	UPROPERTY(EditDefaultsOnly, Category = "Title|Input")
	TObjectPtr<UInputMappingContext> IMC_Hotbar;

private:
	UPROPERTY()
	UUserWidget* TitleRootWidget;

	void RegisterTitleInputContexts();
};