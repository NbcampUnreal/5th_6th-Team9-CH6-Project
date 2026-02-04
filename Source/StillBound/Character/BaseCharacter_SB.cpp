

#include "Character/BaseCharacter_SB.h"
#include "AbilitySystemComponent.h"
#include "Inventory/InventoryComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"



ABaseCharacter_SB::ABaseCharacter_SB()
{
	PrimaryActorTick.bCanEverTick = false;

	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// ASC
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(false);

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

void ABaseCharacter_SB::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("[InitStats] ASC=%d DT=%s AttrClass=%s"),
		AbilitySystemComponent != nullptr,
		*GetNameSafe(DefaultAttributeMetaDataTable),
		*GetNameSafe(AttributeSetClassForInitStats));

	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	if (DefaultAttributeMetaDataTable && AttributeSetClassForInitStats)
	{
		AbilitySystemComponent->InitStats(AttributeSetClassForInitStats, DefaultAttributeMetaDataTable);
	}

	GiveStartupAbilities();

	//스테미너 자동회복
	if (DefaultStaminaRegenEffect)
	{
		FGameplayEffectContextHandle Context =
			AbilitySystemComponent->MakeEffectContext();

		FGameplayEffectSpecHandle Spec =
			AbilitySystemComponent->MakeOutgoingSpec(
				DefaultStaminaRegenEffect,
				1.f,
				Context
			);

		if (Spec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(
				*Spec.Data.Get()
			);
		}
	}
}


UAbilitySystemComponent* ABaseCharacter_SB::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UPlayerAttributeSet* ABaseCharacter_SB::GetPlayerAttributeSet() const
{
	return PlayerAttributeSet;
}

void ABaseCharacter_SB::GiveStartupAbilities()
{
	if (bAbilitiesGiven) return;

	if (!IsValid(GetAbilitySystemComponent())) return;

	for (const auto& Ability : StartupAbilities)
	{
		if (!Ability) continue;
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, 1));
	}

	bAbilitiesGiven = true;
}



