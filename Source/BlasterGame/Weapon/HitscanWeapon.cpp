// (c) 2023 Will Roberts


#include "HitscanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "BlasterGame/Character/BlasterCharacter.h"

void AHitscanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// Cache owner.
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	AController* InstigatorController = OwnerPawn->GetController();
	if (!InstigatorController) return;

	// Perform a line trace.
	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleSocket)
	{
		FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25f; // Extend trace vector by 25%.

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel(FireHit, Start, End, ECollisionChannel::ECC_Visibility);
			if (FireHit.bBlockingHit)
			{
				// Apply damage if a character was hit.
				ABlasterCharacter* HitChar = Cast<ABlasterCharacter>(FireHit.GetActor());
				if (HitChar && HasAuthority())
				{
					UGameplayStatics::ApplyDamage(HitChar, Damage, InstigatorController, this, UDamageType::StaticClass());
				}

				// Play impact FX.
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
				}
			}
		}
	}
}