// (c) 2023 Will Roberts

#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "BlasterGame/Character/BlasterCharacter.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	// Handles animations and ammo updates.
	AWeapon::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (!MuzzleSocket) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// Determine start and end vectors.
	FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
	FVector Start = SocketTransform.GetLocation();
	for (uint32 i = 0; i < ProjectileCount; i++)
	{
		FVector End = TraceWithSpread(Start, HitTarget);
	}
}