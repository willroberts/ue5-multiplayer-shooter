// (c) 2023 Will Roberts

#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "BlasterGame/Character/BlasterCharacter.h"

void AShotgun::MultiFire(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());

	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (!MuzzleSocket) return;

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	// Determine start and end vectors.
	FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
	FVector Start = SocketTransform.GetLocation();

	AController* InstigatorController = OwnerPawn->GetController();
	TMap<ABlasterCharacter*, uint32> HitMap; // Tracks hits per enemy character.
	for (FVector_NetQuantize HitTarget : HitTargets)
	{
		FHitResult FireHit;
		TraceHit(Start, HitTarget, FireHit);

		ABlasterCharacter* HitChar = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (HitChar)
		{
			if (HitMap.Contains(HitChar)) HitMap[HitChar]++;
			else HitMap.Emplace(HitChar, 1);
		}

		// Play impact FX.
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
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
				HitPair.Key, // Hit character.
				Damage * HitPair.Value, // Scale damage by hit count.
				InstigatorController,
				this,
				UDamageType::StaticClass()
			);
		}
	}
}

// MultiTraceWithSpread is similar to AWeapon::TraceWithSpread but has multiple traces/shots.
void AShotgun::MultiTraceWithSpread(const FVector& HitTarget, TArray<FVector_NetQuantize>& Targets)
{
	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (!MuzzleSocket) return;

	// Determine start and end vectors.
	const FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	// Apply spread.
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * SpreadTraceDistance;
	for (uint32 i = 0; i < ProjectileCount; i++)
	{
		const FVector RandomVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SpreadRadius);
		const FVector EndLocation = SphereCenter + RandomVec;
		const FVector ToEndLocation = EndLocation - TraceStart;
		const FVector WithSpread = FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size());

		Targets.Add(WithSpread);
	}
}