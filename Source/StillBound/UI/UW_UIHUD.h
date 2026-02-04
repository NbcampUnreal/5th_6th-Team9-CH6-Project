#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_UIHUD.generated.h"

class UUW_HPBar;
class UUW_StaminaBar;
class UUW_ExpBar;
class UUW_Minimap;
/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_UIHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void SetHP(float Current, float Max);

	UFUNCTION(BlueprintCallable)
	void SetStamina(float Current, float Max);

	UFUNCTION(BlueprintCallable)
	void SetExp(float Current, float Required);

	UFUNCTION(BlueprintCallable)
	void SetLevel(int32 Level);

	UUW_Minimap* GetMiniMapWidget() const { return MiniMapWidget; }

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_HPBar> HPBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_StaminaBar> StaminaBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_ExpBar> ExpBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_Minimap> MiniMapWidget;
};
