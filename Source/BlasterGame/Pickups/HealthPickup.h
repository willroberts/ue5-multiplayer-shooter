// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

UCLASS()
class BLASTERGAME_API AHealthPickup : public APickup
{
	GENERATED_BODY()

public:
	AHealthPickup();
	virtual void Destroyed() override;

protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;

private:
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float HealAmount = 100.f;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	float HealDuration = 3.f;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	class UNiagaraComponent* PickupFX;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	class UNiagaraSystem* DestroyFX;
};