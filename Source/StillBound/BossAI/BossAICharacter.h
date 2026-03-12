
#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter_SB.h"
#include "BossAICharacter.generated.h"

class UBosAIAttributeSet;

UCLASS()
class STILLBOUND_API ABossAICharacter : public ABaseCharacter_SB
{
	GENERATED_BODY()

public:
	ABossAICharacter();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Attributes")
	TObjectPtr<UBosAIAttributeSet> BossAttributeSet;
};
