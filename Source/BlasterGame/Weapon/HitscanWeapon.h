// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitscanWeapon.generated.h"

UCLASS()
class BLASTERGAME_API AHitscanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

protected:
	FVector TraceWithSpread(const FVector& TraceStart, const FVector& HitTarget);

private:
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bUseSpread = false;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float SpreadTraceDistance = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float SpreadRadius = 50.f;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* SmokeBeamParticles;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* MuzzleFlashParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* FireSound;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;
};