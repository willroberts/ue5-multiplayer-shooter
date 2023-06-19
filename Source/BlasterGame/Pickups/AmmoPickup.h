// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "BlasterGame/Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"

UCLASS()
class BLASTERGAME_API AAmmoPickup : public APickup
{
	GENERATED_BODY()

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
	EWeaponType AmmoType;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	int32 AmmoAmount = 30;
};