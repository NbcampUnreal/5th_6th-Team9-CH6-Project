
#pragma once

#include "CoreMinimal.h"
#include "Character/SB_BaseCharacter.h"
#include "SB_PlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class STILLBOUND_API ASB_PlayerCharacter : public ASB_BaseCharacter
{
	GENERATED_BODY()
	
public:
	ASB_PlayerCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void PossessedBy(AController* NewController) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;
};
