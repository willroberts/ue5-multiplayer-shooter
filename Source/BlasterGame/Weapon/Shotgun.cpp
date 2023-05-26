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

	AController* InstigatorController = OwnerPawn->GetController();
	TMap<ABlasterCharacter*, uint32> HitMap;
	for (uint32 i = 0; i < ProjectileCount; i++)
	{
		FHitResult FireHit;
		TraceHit(Start, HitTarget, FireHit);

		ABlasterCharacter* HitChar = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (HasAuthority() && InstigatorController && HitChar)
		{
			if (HitMap.Contains(HitChar)) HitMap[HitChar]++;
			else HitMap.Emplace(HitChar, 1);
		}

		// Play impact FX.
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
		}
		if (ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, FireHit.ImpactPoint, 0.5f, FMath::FRandRange(-0.5f, 0.5f));
		}
	}

	// Apply damage if a character was hit.
	for (auto HitPair : HitMap)
	{
		if (HasAuthority() && HitPair.Key && HitPair.Value > 0 && InstigatorController)
		{
			UGameplayStatics::ApplyDamage(
				HitPair.Key,
				Damage * HitPair.Value,
				InstigatorController,
				this,
				UDamageType::StaticClass()
			);
		}
	}
}