
#include "BossAI/BossAICharacter.h"
#include "BossAI/BosAIAttributeSet.h"

ABossAICharacter::ABossAICharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// 플레이어도 같은 식으로 CreateDefaultSubobject + AttributeSetClassForInitStats 를 같이 씀
	BossAttributeSet = CreateDefaultSubobject<UBosAIAttributeSet>(TEXT("BossAttributeSet"));
	AttributeSetClassForInitStats = UBosAIAttributeSet::StaticClass();
}

void ABossAICharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!AbilitySystemComponent) return;

	const float H = AbilitySystemComponent->GetNumericAttribute(UBosAIAttributeSet::GetHealthAttribute());
	const float MH = AbilitySystemComponent->GetNumericAttribute(UBosAIAttributeSet::GetMaxHealthAttribute());

	UE_LOG(LogTemp, Warning, TEXT("[Boss] After InitStats H=%.1f / %.1f"), H, MH);
}