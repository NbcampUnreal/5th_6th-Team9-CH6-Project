#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SBTitlePlayerController.generated.h"

UCLASS()
class STILLBOUND_API ASBTitlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Title|UI")
	TSubclassOf<UUserWidget> TitleRootWidgetClass;

private:
	UPROPERTY()
	UUserWidget* TitleRootWidget;
};