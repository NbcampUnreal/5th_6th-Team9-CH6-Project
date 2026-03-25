
#include "BossAI/BosAIAttributeSet.h"
#include "BossAI/BossAICharacter.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/PlayerController_SB.h"
#include "UI/USB_UIManager.h"
#include "GameplayTagContainer.h"

void UBosAIAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}

	ClampAttribute(Attribute, NewValue);
}

void UBosAIAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute& AffectedAttr = Data.EvaluatedData.Attribute;

	if (AffectedAttr == GetDamageAttribute())
	{
		const float LocalDamage = GetDamage();
		SetDamage(0.f);

		if (LocalDamage > 0.f)
		{
			const float Reduced = FMath::Max(LocalDamage - GetDefense(), 0.f);
			const float NewHealth = FMath::Clamp(GetHealth() - Reduced, 0.f, GetMaxHealth());
			SetHealth(NewHealth);

			AActor* Owner = GetOwningActor();
			if (!Owner) return;

			if (ABossAICharacter* Boss = Cast<ABossAICharacter>(Owner))
			{
				Boss->ShowDamageNumber(Reduced);
			}

			APlayerController_SB* PC = Cast<APlayerController_SB>(GetWorld()->GetFirstPlayerController());
			if (PC && PC->UIManager)
			{
				PC->UIManager->UpdateBossHP(NewHealth, GetMaxHealth());
			}

			if (Reduced > 0.f && NewHealth > 0.f)
			{
				const FGameplayTag GetHitEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Boss.GetHit"));

				FGameplayEventData HitEvent;
				HitEvent.EventTag = GetHitEventTag;
				HitEvent.Target = Owner;
				HitEvent.EventMagnitude = Reduced;
				HitEvent.Instigator = Data.EffectSpec.GetContext().GetInstigator();
				HitEvent.OptionalObject = Data.EffectSpec.GetContext().GetSourceObject();

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, GetHitEventTag, HitEvent);
			}

			if (NewHealth <= 0.f)
			{
				if (APlayerController_SB* BossPC = Cast<APlayerController_SB>(GetWorld()->GetFirstPlayerController()))
				{
					if (BossPC->UIManager)
					{
						BossPC->UIManager->HideBossHP();
						BossPC->UIManager->ShowGameClear(true);
					}
				}

				if (!Owner->HasAuthority()) return;

				if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner))
				{
					const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(TEXT("Boss.State.Dead"));
					const FGameplayTag DeathEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Boss.Death"));

					if (!ASC->HasMatchingGameplayTag(DeadTag))
					{
						ASC->AddLooseGameplayTag(DeadTag);

						FGameplayEventData EventData;
						EventData.EventTag = DeathEventTag;
						EventData.Target = Owner;

						UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, DeathEventTag, EventData);
					}
				}
			}
		}
	}

	SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	SetAttack(FMath::Max(GetAttack(), 0.f));
	SetDefense(FMath::Max(GetDefense(), 0.f));
	SetPhase(FMath::Max(GetPhase(), 1.f));
}

void UBosAIAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetAttackAttribute() || Attribute == GetDefenseAttribute() || Attribute == GetPhaseAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}