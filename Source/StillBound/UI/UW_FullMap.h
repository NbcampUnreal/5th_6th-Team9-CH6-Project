#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_FullMap.generated.h"

class UImage;
class UCanvasPanel;
/**
 * 
 */
UCLASS()
class STILLBOUND_API UUW_FullMap : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;

	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	void UpdatePlayerPosition(const FVector2D& PlayerUV);

	void UpdatePing(const FVector2D& PingUV);
	void ClearPing();

protected:

	UPROPERTY(meta =(BindWidget))
	TObjectPtr< UCanvasPanel> MapRoot;

	UPROPERTY(meta =(BindWidget))
	TObjectPtr< UCanvasPanel> MapContent;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MapImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Ping")
	TSubclassOf<UUserWidget> PingWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> ActivePing;

private:

float CurrentZoom = 1.f;

	UPROPERTY(EditAnywhere)
	float MinZoom = 1.f;

	UPROPERTY(EditAnywhere)
	float MaxZoom = 5.f;

	UPROPERTY(EditAnywhere)
	float ZoomStep = 0.3f;

	FVector2D PanOffset = FVector2D::ZeroVector;

	bool bIsDragging = false;
	FVector2D LastMousePosition = FVector2D::ZeroVector;

	void ClampPan();
	void ApplyViewTransform();

};
