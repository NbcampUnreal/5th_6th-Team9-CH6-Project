#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuildEntryWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UTexture2D;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBuildEntryClicked, FName);


UCLASS()
class STILLBOUND_API UBuildEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitEntry(FName InBuildingID, UTexture2D* InIcon, const FText& InDisPlayName);

	FOnBuildEntryClicked OnBuildEntryClicked;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> EntryButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> BuildingIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BuildingNameText;

private:
	UFUNCTION()
	void HandleClicked();

	FName BuildingID = NAME_None;
};
