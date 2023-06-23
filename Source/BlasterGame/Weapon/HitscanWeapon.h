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
	void TraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHitResult);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class USoundCue* ImpactSound;

private:
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UParticleSystem* SmokeBeamParticles;
};