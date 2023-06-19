// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

UCLASS()
class BLASTERGAME_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuffComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void RestoreHealth(float Amount, float Duration);

protected:
	virtual void BeginPlay() override;
	void RestoreHealthOverTime(float DeltaTime);

private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	bool bHealing = false;
	float HealRate = 0;
	float HealAmount = 0.f;
};