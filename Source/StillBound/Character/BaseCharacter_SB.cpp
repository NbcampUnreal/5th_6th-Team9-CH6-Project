

#include "Character/BaseCharacter_SB.h"
#include "AbilitySystemComponent.h"
#include "Character/PlayerAttributeSet.h"

ABaseCharacter_SB::ABaseCharacter_SB()
{
	PrimaryActorTick.bCanEverTick = false;

	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// ASC
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(false);

	// AttributeSet
	PlayerAttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("PlayerAttributeSet"));
}

void ABaseCharacter_SB::BeginPlay()
{
	Super::BeginPlay();

	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	if (DefaultAttributeMetaDataTable)
	{
		AbilitySystemComponent->InitStats(UPlayerAttributeSet::StaticClass(), DefaultAttributeMetaDataTable);
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



