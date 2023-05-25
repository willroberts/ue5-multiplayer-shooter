// (c) 2023 Will Roberts


#include "HitscanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "BlasterGame/Character/BlasterCharacter.h"

void AHitscanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (!MuzzleSocket) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// Determine start and end vectors.
	FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
	FVector Start = SocketTransform.GetLocation();
	FVector End = Start + (HitTarget - Start) * 1.25f; // Extend trace vector by 25%.
	FVector BeamEnd = End;

	// Perform a line trace to the target.
	FHitResult FireHit;
	World->LineTraceSingleByChannel(FireHit, Start, End, ECollisionChannel::ECC_Visibility);
	if (FireHit.bBlockingHit)
	{
		BeamEnd = FireHit.ImpactPoint;

		// Apply damage if a character was hit.
		AController* InstigatorController = OwnerPawn->GetController();
		ABlasterCharacter* HitChar = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (InstigatorController && HitChar && HasAuthority())
		{
			UGameplayStatics::ApplyDamage(HitChar, Damage, InstigatorController, this, UDamageType::StaticClass());
		}

		// Play impact FX.
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticles, BeamEnd, FireHit.ImpactNormal.Rotation());
		}
		if (ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, BeamEnd);
		}
	}

	if (MuzzleFlashParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, MuzzleFlashParticles, SocketTransform);
	}

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	if (SmokeBeamParticles)
	{
		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, SmokeBeamParticles, SocketTransform);
		if (Beam) Beam->SetVectorParameter(FName("Target"), BeamEnd);
	}
}