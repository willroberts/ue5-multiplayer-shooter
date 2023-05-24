// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTERGAME_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	// This RPC must be Reliable, since Projectiles are destroyed on hit.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastHitFX(FVector_NetQuantize HitLocation, bool bHitPlayer);

	/*
	* Gameplay attributes
	*/

	UPROPERTY(EditAnywhere)
	float Damage = 12.5f;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	/*
	* Impact FX
	*/

	UPROPERTY(EditAnywhere)
	class UParticleSystem* SolidImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* SolidImpactSound;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* PlayerImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* PlayerImpactSound;

private:
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;
};