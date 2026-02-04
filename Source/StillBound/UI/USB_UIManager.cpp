#include "UI/USB_UIManager.h"
#include "UI/UW_UIHUD.h"
#include "UI/UW_Minimap.h"
#include "GameFramework/PlayerController.h"
#include "Character/BaseCharacter_SB.h"
#include "Character/PlayerCharacter_SB.h"
#include "Character/PlayerAttributeSet.h"

void USB_UIManager::Init(APlayerController* InOwnerPC)
{
	OwnerPC = InOwnerPC;
	if (!OwnerPC || !UIHUDClass) return;

	UIHUD = CreateWidget<UUW_UIHUD>(OwnerPC, UIHUDClass);
	if (!UIHUD) return;

	UIHUD->AddToViewport(10);

	ABaseCharacter_SB* Char = Cast<ABaseCharacter_SB>(OwnerPC->GetPawn());
	if (!Char) return;

	if (APlayerCharacter_SB* Player = Cast<APlayerCharacter_SB>(Char))
	{
		if (UIHUD->GetMiniMapWidget() && Player->GetMiniMapTarget())
		{
			UIHUD->GetMiniMapWidget()->SetMiniMapTexture(Player->GetMiniMapTarget());
		}
	}

	UPlayerAttributeSet* AS = Char->GetPlayerAttributeSet();
	if (!AS) return;

	AS->OnExperienceChanged.AddDynamic(this, &USB_UIManager::OnExpChanged);
	AS->OnLevelChanged.AddDynamic(this, &USB_UIManager::OnLevelChanged);

	UpdateHUD();
}

void USB_UIManager::SetHP(float Current, float Max)
{
	if (!UIHUD) return;

	UIHUD->SetHP(Current, Max);
}

void USB_UIManager::SetStamina(float Current, float Max)
{
	if (!UIHUD) return;
	UIHUD->SetStamina(Current, Max); 
}

void USB_UIManager::SetExp(float Current, float Required)
{
	if (!UIHUD) return;
	UIHUD->SetExp(Current, Required);
}

void USB_UIManager::SetLevel(int32 Level)
{
	if (!UIHUD) return;
	UIHUD->SetLevel(Level);
}

void USB_UIManager::UpdateHUD()
{
	if (!OwnerPC || !UIHUD) return;

	ABaseCharacter_SB* Char = Cast<ABaseCharacter_SB>(OwnerPC->GetPawn());
	if (!Char) return;

	UPlayerAttributeSet* AS = Char->GetPlayerAttributeSet();
	if (!AS) return;

	UIHUD->SetHP(AS->GetHealth(), AS->GetMaxHealth());
	UIHUD->SetStamina(AS->GetStamina(), AS->GetMaxStamina());

	const float CurExp = AS->GetExperience();
	const int32 Level = AS->GetLevel();
	const float RequiredExp = AS->GetRequiredExpForLevel(Level);

	UIHUD->SetExp(CurExp, RequiredExp);
	UIHUD->SetLevel(Level);
}

void USB_UIManager::OnExpChanged(float OldValue, float NewValue)
{
	UpdateHUD();
}

void USB_UIManager::OnLevelChanged(float OldValue, float NewValue)
{
	UpdateHUD();
}
