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
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	uint32 ProjectileCount = 8;
};