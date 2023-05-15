// (c) 2023 Will Roberts

#include "ProjectileWeapon.h"

#include "Engine/SkeletalMeshSocket.h"

#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// Firing a weapon can only occur on the server (followed by replication to clients).
	// Can't spawn projectile if parent class is nullptr.
	if (!HasAuthority() || !ProjectileClass)
	{
		return;
	}

	// Use the character as the instigator pawn.
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	if (!InstigatorPawn)
	{
		return;
	}

	// Get the "MuzzleFlash" socket location.
	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (!MuzzleSocket)
	{
		// Can't spawn projectile without an origin location.
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Get weapon transform and rotation.
	FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
	FVector ToTarget = HitTarget - SocketTransform.GetLocation(); // Vector from socket to trace target.

	// Configure spawn parameters.
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = InstigatorPawn;

	// Spawn a projectile at the 'MuzzleFlash' socket.
	World->SpawnActor<AProjectile>(
		ProjectileClass,
		SocketTransform.GetLocation(),
		ToTarget.Rotation(),
		SpawnParams
	);
}