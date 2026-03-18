#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_BossHPbar.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 
 */
UCLASS()
class STILLBOUND_API UBossHPbar : public UUserWidget
{
	GENERATED_BODY()
	
public: 
	virtual void NativeConstruct() override;

	void SetBossName(const FText& Name);
	void SetHPPercent(float Percent);

protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HPBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BossName;

};
