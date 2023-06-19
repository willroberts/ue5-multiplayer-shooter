// (c) 2023 Will Roberts

#include "HealthPickup.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "BlasterGame/Character/BlasterCharacter.h"
#include "BlasterGame/Components/BuffComponent.h"

AHealthPickup::AHealthPickup()
{
	bReplicates = true;

	PickupFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	PickupFX->SetupAttachment(RootComponent);
}

void AHealthPickup::Destroyed()
{
	if (DestroyFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, DestroyFX, GetActorLocation(), GetActorRotation());
	}

	Super::Destroyed();
}

void AHealthPickup::OnSphereOverlap(
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
		if (Buff)
		{
			Buff->RestoreHealth(HealAmount, HealDuration);
		}
	}

	Destroy();
}