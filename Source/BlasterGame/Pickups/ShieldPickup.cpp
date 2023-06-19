// (c) 2023 Will Roberts

#include "ShieldPickup.h"
#include "BlasterGame/Character/BlasterCharacter.h"
#include "BlasterGame/Components/BuffComponent.h"

AShieldPickup::AShieldPickup()
{
	bReplicates = true;
}

void AShieldPickup::OnSphereOverlap(
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
		UBuffComponent* Buff = Char->GetBuffComponent();
		if (Buff) Buff->RestoreShield(RestoreAmount, RestoreDuration);
	}

	Destroy();
}