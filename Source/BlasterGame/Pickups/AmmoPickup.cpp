// (c) 2023 Will Roberts

#include "AmmoPickup.h"
#include "BlasterGame/Character/BlasterCharacter.h"
#include "BlasterGame/Components/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* Char = Cast<ABlasterCharacter>(OtherActor);
	if (Char)
	{
		UCombatComponent* Combat = Char->GetCombatComponent();
		if (Combat)
		{
			Combat->AddAmmo(AmmoType, AmmoAmount);
		}
	}
	Destroy();
}