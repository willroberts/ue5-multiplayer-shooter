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
	void ApplySpeedBuff(float Magnitude, float Duration);
	void ApplyJumpBuff(float Magnitude, float Duration);

protected:
	virtual void BeginPlay() override;
	void RestoreHealthOverTime(float DeltaTime);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float Magnitude);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float Magnitude);

private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	/*
	* Health Pickup
	*/

	bool bHealing = false;
	float HealRate = 0;
	float HealAmount = 0.f;

	/*
	* Speed Pickup
	*/

	FTimerHandle SpeedBuffTimer;
	void SpeedBuffTimerFinished();

	/*
	* Jump Pickup
	*/

	FTimerHandle JumpBuffTimer;
	void JumpBuffTimerFinished();
};