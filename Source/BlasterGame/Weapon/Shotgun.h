// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "HitscanWeapon.h"
#include "Shotgun.generated.h"

UCLASS()
class BLASTERGAME_API AShotgun : public AHitscanWeapon
{
	GENERATED_BODY()

public:
	virtual void MultiFire(const TArray<FVector_NetQuantize>& HitTargets);
	void MultiTraceWithSpread(const FVector& HitTarget, TArray<FVector_NetQuantize>& Targets);

private:
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	uint32 ProjectileCount = 8;
};