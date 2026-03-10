#include "UI/DamageNumberActor.h"
#include "Components/WidgetComponent.h"
#include "UI/UW_DamageText.h"

ADamageNumberActor::ADamageNumberActor()
{
	PrimaryActorTick.bCanEverTick = false;

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidget"));
	RootComponent = WidgetComponent;

	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetDrawSize(FVector2D(200, 100));
}

void ADamageNumberActor::BeginPlay()
{
	Super::BeginPlay();

	if (DamageTextWidgetClass)
	{
		WidgetComponent->SetWidgetClass(DamageTextWidgetClass);
	}

	SetLifeSpan(1.2f);
}

void ADamageNumberActor::InitDamage(float Damage)
{
	if (!WidgetComponent) return;

	UUserWidget* Widget = WidgetComponent->GetUserWidgetObject();
	if (!Widget) return;

	UUW_DamageText* DamageWidget = Cast<UUW_DamageText>(Widget);

	if (DamageWidget)
	{
		DamageWidget->SetDamage(FMath::RoundToInt(Damage));
	}
}

