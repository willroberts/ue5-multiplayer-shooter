// © 2023 Will Roberts

#include "ProjectileWeapon.h"

#include "Engine/SkeletalMeshSocket.h"

#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// Firing a weapon can only occur on the server (followed by replication to clients).
	if (!HasAuthority())
	{
		return;
	}

	// Can't spawn projectile if parent class is nullptr.
	if (!ProjectileClass)
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
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (!MuzzleFlashSocket)
	{
		// Can't spawn projectile without an origin location.
		return;
	}

	// Get weapon transform and rotation.
	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	FVector ToTarget = HitTarget - SocketTransform.GetLocation(); // Vector from socket to trace target.
	FRotator TargetRotation = ToTarget.Rotation();

	// Configure spawn parameters.
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = InstigatorPawn;

	// Spawn a projectile at the 'MuzzleFlash' socket.
	UWorld* World = GetWorld();
	if (World)
	{
		World->SpawnActor<AProjectile>(
			ProjectileClass,
			SocketTransform.GetLocation(),
			TargetRotation,
			SpawnParams
		);
	}
}