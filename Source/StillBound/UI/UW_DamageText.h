#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_DamageText.generated.h"

class UTextBlock;
class UWidgetAnimation;

/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_DamageText : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void SetDamage(int32 Damage);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Damage;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> Anim_Float;

};
