

#include "AI/AIAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "AI/EnemyCharacter.h"


void UAIAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
		NewValue = FMath::Max(NewValue, 1.f);

	if (Attribute == GetLevelAttribute())
	{
		NewValue = FMath::Max(1.f, FMath::FloorToFloat(NewValue));
	}

	ClampAttribute(Attribute, NewValue);
}

void UAIAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute& AffectedAttr = Data.EvaluatedData.Attribute;

	if (AffectedAttr == GetDamageAttribute())
	{
		const float LocalDamage = GetDamage();
		SetDamage(0.f);

		UE_LOG(LogTemp, Warning, TEXT("[AIAttr][Auth=%d] DamageAttr triggered. LocalDamage=%.1f (after reset Dmg=%.1f)"),
			GetOwningActor() ? GetOwningActor()->HasAuthority() : -1,
			LocalDamage, GetDamage()
		);

		if (LocalDamage > 0.f)
		{
			const float Reduced = FMath::Max(LocalDamage - GetDefense(), 0.f);
			const float OldHealth = GetHealth();
			const float MaxH = GetMaxHealth();
			const float NewHealth = FMath::Clamp(GetHealth() - Reduced, 0.f, GetMaxHealth());
			SetHealth(NewHealth);

			UE_LOG(LogTemp, Warning,
				TEXT("[AIAttr][Auth=%d][This=%p] DamageIn=%.1f  H: %.1f/%.1f  Owning=%s"),
				GetOwningActor() ? GetOwningActor()->HasAuthority() : -1,
				this,
				LocalDamage,
				OldHealth, MaxH,
				*GetNameSafe(GetOwningActor())
			);

			if (Reduced > 0.f && NewHealth > 0.f)
			{
				AActor* Owner = GetOwningActor();
				if (Owner)
				{

					const FGameplayTag GetHitEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Enemy.GetHit"));

					FGameplayEventData HitEvent;
					HitEvent.EventTag = GetHitEventTag;
					HitEvent.Target = Owner;
					HitEvent.EventMagnitude = Reduced;
					HitEvent.Instigator = Data.EffectSpec.GetContext().GetInstigator();
					HitEvent.OptionalObject = Data.EffectSpec.GetContext().GetSourceObject();

					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, GetHitEventTag, HitEvent);
				}
			}

			if (NewHealth <= 0.f)
			{

				UE_LOG(LogTemp, Error, TEXT("[AIAttr][Auth=%d] Health <= 0, calling HandleDeath()"),
					GetOwningActor() ? GetOwningActor()->HasAuthority() : -1
				);

				AActor* Owner = GetOwningActor();
				if (!Owner || !Owner->HasAuthority())
				{
					return;
				}

				UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
				if (!ASC)
				{
					return;
				}

				const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(TEXT("Enemy.State.Dead"));
				const FGameplayTag DeathEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Enemy.Death"));

				if (ASC->HasMatchingGameplayTag(DeadTag))
				{
					return;
				}

				ASC->AddLooseGameplayTag(DeadTag);

				FGameplayEventData EventData;
				EventData.EventTag = DeathEventTag;
				EventData.Target = Owner;

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, DeathEventTag, EventData);
			}
		}
	}

	SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	SetAttack(FMath::Max(GetAttack(), 0.f));
	SetDefense(FMath::Max(GetDefense(), 0.f));
	SetExpReward(FMath::Max(GetExpReward(), 0.f));
}

void UAIAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	else if (Attribute == GetAttackAttribute() || Attribute == GetDefenseAttribute() || Attribute == GetExpRewardAttribute())
		NewValue = FMath::Max(NewValue, 0.f);
}