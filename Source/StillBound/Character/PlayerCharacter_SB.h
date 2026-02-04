
#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter_SB.h"
#include "PlayerCharacter_SB.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

UCLASS()
class STILLBOUND_API APlayerCharacter_SB : public ABaseCharacter_SB
{
	GENERATED_BODY()
	
public:
	APlayerCharacter_SB();

	UTextureRenderTarget2D* GetMiniMapTarget() const { return MiniMapTarget; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	//Minimap Camera
	UPROPERTY(VisibleAnywhere, Category = "MiniMap")
	TObjectPtr<USpringArmComponent> MiniMapArm;

	UPROPERTY(VisibleAnywhere, Category = "MiniMap")
	TObjectPtr<USceneCaptureComponent2D> MiniMapCapture;

	UPROPERTY(EditDefaultsOnly, Category = "MiniMap")
	TObjectPtr<UTextureRenderTarget2D> MiniMapTarget;
};
