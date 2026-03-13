#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Public/Data/BuildingData.h"
#include "BuildMenuWidget.generated.h"

class UButton;
class UScrollBox;
class UWrapBox;
class UBuildEntryWidget;
class UDataTable;
class UBuildComponent;

UCLASS()
class STILLBOUND_API UBuildMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void Init(UBuildComponent* InBuildComponent);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Structure;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Production;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> EntryScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> EntryWrapBox;

	UPROPERTY(EditDefaultsOnly, Category = "Build|UI")
	TSubclassOf<UBuildEntryWidget> BuildEntryClass;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Data")
	TObjectPtr<UDataTable> BuildingDataTable;

	UFUNCTION()
	void OnClickedStructure();

	UFUNCTION()
	void OnClickedProduction();

	void RefreshEntries(EBuildCategory Category);

	void HandleBuildEntryClicked(FName BuildingID);

private:
	UPROPERTY()
	TObjectPtr<UBuildComponent> BuildComponentRef;

	EBuildCategory CurrentCategory = EBuildCategory::Structure;
};
