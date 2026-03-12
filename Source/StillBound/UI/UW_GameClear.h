#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_GameClear.generated.h"

class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_GameClear : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	void SetGameClear(bool bBossKilled);

	UFUNCTION(BlueprintCallable)
	void GoToTitleMenu();

protected:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResultText;

	UPROPERTY(meta = (BindWidget))
	UButton* TitleButton;
};
