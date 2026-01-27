

#include "Character/BaseCharacter_SB.h"
#include "AbilitySystemComponent.h"


ABaseCharacter_SB::ABaseCharacter_SB()
{
	PrimaryActorTick.bCanEverTick = false;

	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// ASC
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(false);
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



