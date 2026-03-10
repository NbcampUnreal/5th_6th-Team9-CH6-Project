#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageNumberActor.generated.h"

class UWidgetComponent;
class UUW_DamageText;

UCLASS()
class STILLBOUND_API ADamageNumberActor : public AActor
{
	GENERATED_BODY()
	

public:
	ADamageNumberActor();

	void InitDamage(float Damage);

protected:
	virtual void BeginPlay() override;

protected:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent> WidgetComponent;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUW_DamageText> DamageTextWidgetClass;
};