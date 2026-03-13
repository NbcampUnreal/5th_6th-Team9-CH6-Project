
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "UW_AIAlert.generated.h"

/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_AIAlert : public UUserWidget
{
	GENERATED_BODY()

public:

    virtual void NativeConstruct() override;

protected:

    UPROPERTY(meta = (BindWidget))
    UImage* AlertImage;
};
