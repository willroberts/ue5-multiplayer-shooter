// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

UCLASS()
class BLASTERGAME_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	virtual void OnHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	) override;

	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* MovementComponent;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* RocketTrailSystem;

	UPROPERTY()
	class UNiagaraComponent* RocketTrailComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* RocketFlightSoundCue;

	UPROPERTY()
	class UAudioComponent* RocketFlightSoundComponent;

	UPROPERTY(EditAnywhere)
	class USoundAttenuation* RocketFlightSoundAttenuation;

	void DestroyTimerFinished();

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.0f;
};