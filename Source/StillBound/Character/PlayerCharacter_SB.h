
#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter_SB.h"
#include "PlayerCharacter_SB.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class STILLBOUND_API APlayerCharacter_SB : public ABaseCharacter_SB
{
	GENERATED_BODY()
	
public:
	APlayerCharacter_SB();

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;
};
