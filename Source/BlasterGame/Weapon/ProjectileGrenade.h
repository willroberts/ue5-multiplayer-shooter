// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

UCLASS()
class BLASTERGAME_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileGrenade();

	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

private:
	UPROPERTY(EditAnywhere)
	class USoundCue* BounceSound;
};