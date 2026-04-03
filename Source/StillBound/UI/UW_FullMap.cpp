#include "UI/UW_FullMap.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Character/PlayerController_SB.h"
#include "Input/Reply.h"

void UUW_FullMap::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;

	CurrentZoom = 1.f;
	PanOffset = FVector2D::ZeroVector;

	if (MapContent)
	{
		MapContent->SetRenderTransformPivot(FVector2D(0.f, 0.f));
	}

	ApplyViewTransform();
}

FReply UUW_FullMap::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!MapRoot) return FReply::Handled();

	const float Wheel = InMouseEvent.GetWheelDelta();

	const float OldZoom = CurrentZoom;
	const float NewZoom = FMath::Clamp(
		OldZoom + Wheel * ZoomStep,
		MinZoom,
		MaxZoom);

	if (FMath::IsNearlyEqual(OldZoom, NewZoom))
		return FReply::Handled();

	const FGeometry RootGeo = MapRoot->GetCachedGeometry();

	const FVector2D LocalMouse =
		RootGeo.AbsoluteToLocal(
			InMouseEvent.GetScreenSpacePosition());

	const FVector2D Size = RootGeo.GetLocalSize();

	FVector2D ScreenUV(
		FMath::Clamp(LocalMouse.X / Size.X, 0.f, 1.f),
		FMath::Clamp(LocalMouse.Y / Size.Y, 0.f, 1.f)
	);

	const float OldView = 1.f / OldZoom;
	const float NewView = 1.f / NewZoom;

	PanOffset += ScreenUV * (OldView - NewView);

	CurrentZoom = NewZoom;

	ClampPan();
	ApplyViewTransform();

	return FReply::Handled();
}

FReply UUW_FullMap::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!MapContent)
		return FReply::Unhandled();

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		const FGeometry ContentGeo =
			MapContent->GetCachedGeometry();

		const FVector2D Local =
			ContentGeo.AbsoluteToLocal(
				InMouseEvent.GetScreenSpacePosition());

		const FVector2D Size =
			ContentGeo.GetLocalSize();

		FVector2D UV;
		UV.X = FMath::Clamp(Local.X / Size.X, 0.f, 1.f);
		UV.Y = FMath::Clamp(Local.Y / Size.Y, 0.f, 1.f);

		UV.Y = 1.f - UV.Y;

		if (APlayerController_SB* PC =
			Cast<APlayerController_SB>(GetOwningPlayer()))
		{
			PC->SetPing(UV);
		}

		return FReply::Handled();
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;
		LastMousePosition =
			InMouseEvent.GetScreenSpacePosition();

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply UUW_FullMap::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bIsDragging || !MapRoot)
		return FReply::Unhandled();

	const FVector2D Current =
		InMouseEvent.GetScreenSpacePosition();

	const FVector2D Delta =
		Current - LastMousePosition;

	LastMousePosition = Current;

	const FVector2D Size =
		MapRoot->GetCachedGeometry().GetLocalSize();

	PanOffset.X -= (Delta.X / Size.X) * (1.f / CurrentZoom);
	PanOffset.Y -= (Delta.Y / Size.Y) * (1.f / CurrentZoom);

	ClampPan();
	ApplyViewTransform();

	return FReply::Handled();
}

FReply UUW_FullMap::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	bIsDragging = false;
	return FReply::Handled();
}

void UUW_FullMap::UpdatePlayerPosition(const FVector2D& PlayerUV)
{
	if (!PlayerIcon)
		return;

	UCanvasPanelSlot* CanvasSlot =
		Cast<UCanvasPanelSlot>(PlayerIcon->Slot);

	if (!CanvasSlot)
		return;

	FVector2D UV = PlayerUV;
	UV.Y = 1.f - UV.Y;

	const FVector2D Size =
		MapContent->GetCachedGeometry().GetLocalSize();

	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));

	CanvasSlot->SetPosition(FVector2D(
		UV.X * Size.X,
		UV.Y * Size.Y
	));
}

void UUW_FullMap::ClampPan()
{
	const float ViewSize = 1.f / CurrentZoom;

	PanOffset.X = FMath::Clamp(PanOffset.X, 0.f, 1.f - ViewSize);
	PanOffset.Y = FMath::Clamp(PanOffset.Y, 0.f, 1.f - ViewSize);
}

void UUW_FullMap::ApplyViewTransform()
{
	if (!MapRoot || !MapContent)
		return;

	const FVector2D Size =
		MapRoot->GetCachedGeometry().GetLocalSize();

	MapContent->SetRenderScale(
		FVector2D(CurrentZoom, CurrentZoom));

	const FVector2D ShiftPx(
		-PanOffset.X * Size.X * CurrentZoom,
		-PanOffset.Y * Size.Y * CurrentZoom
	);

	MapContent->SetRenderTranslation(ShiftPx);
}

void UUW_FullMap::UpdatePing(const FVector2D& PingUV)
{
	if (!MapContent)
		return;

	if (!ActivePing)
	{
		if (!PingWidgetClass)
			return;

		ActivePing =
			CreateWidget<UUserWidget>(
				GetWorld(),
				PingWidgetClass);

		MapContent->AddChild(ActivePing);
	}

	UCanvasPanelSlot* CanvasSlot =
		Cast<UCanvasPanelSlot>(ActivePing->Slot);

	if (!CanvasSlot)
		return;

	FVector2D UV = PingUV;
	UV.Y = 1.f - UV.Y;

	const FVector2D Size =
		MapContent->GetCachedGeometry().GetLocalSize();

	CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));

	CanvasSlot->SetPosition(FVector2D(
		UV.X * Size.X,
		UV.Y * Size.Y
	));
}

void UUW_FullMap::ClearPing()
{
	if (ActivePing)
	{
		ActivePing->RemoveFromParent();
		ActivePing = nullptr;
	}
}
