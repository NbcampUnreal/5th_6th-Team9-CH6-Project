

#include "AI/AIAttributeSet.h"
#include "GameplayEffectExtension.h"
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


	UE_LOG(LogTemp, Warning, TEXT("[AIAttr][Auth=%d] PostGEExecute: Attr=%s  H=%.1f/%.1f  Dmg=%.1f Def=%.1f"),
		GetOwningActor() ? GetOwningActor()->HasAuthority() : -1,
		*AffectedAttr.GetName(),
		GetHealth(), GetMaxHealth(),
		GetDamage(), GetDefense()
	);

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

			if (NewHealth <= 0.f)
			{

				UE_LOG(LogTemp, Error, TEXT("[AIAttr][Auth=%d] Health <= 0, calling HandleDeath()"),
					GetOwningActor() ? GetOwningActor()->HasAuthority() : -1
				);

				if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(GetOwningActor()))
				{
					Enemy->HandleDeath();
				}
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