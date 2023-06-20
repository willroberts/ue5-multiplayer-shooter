// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class BLASTERGAME_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void BindOverlapTimerFinished();

	UPROPERTY(EditAnywhere, Category = "Pickup")
	float RotationSpeed = 45.f; // Degrees per second.

private:
	UPROPERTY(EditAnywhere, Category = "Pickup")
	UStaticMeshComponent* PickupMesh;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	class USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	class UNiagaraComponent* PickupFX;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	class UNiagaraSystem* DestroyFX;

	FTimerHandle BindOverlapTimer;
	float BindOverlapDelay = 0.25f;
};