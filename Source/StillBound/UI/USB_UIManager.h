#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "USB_UIManager.generated.h"

class APlayerController;
class UUW_UIHUD;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class STILLBOUND_API USB_UIManager : public UObject
{
	GENERATED_BODY()
	
public:

	void Init(APlayerController* InOwnerPC);

	void SetHP(float Current, float Max);

	void UpdateHUD();
private:

	UPROPERTY()
	TObjectPtr<APlayerController> OwnerPC;

	UPROPERTY()
	TObjectPtr<UUW_UIHUD> UIHUD;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUW_UIHUD> UIHUDClass;

};
