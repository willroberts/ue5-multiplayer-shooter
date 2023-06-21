// (c) 2023 Will Roberts

#include "HitscanWeapon.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "BlasterGame/Character/BlasterCharacter.h"
#include "BlasterGame/Weapon/WeaponTypes.h"

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

	// Perform a line trace to the target.
	FHitResult FireHit;
	TraceHit(Start, HitTarget, FireHit);

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
		UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, FireHit.ImpactPoint);
	}
}

FVector AHitscanWeapon::TraceWithSpread(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * SpreadTraceDistance;
	FVector RandomVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SpreadRadius);
	FVector EndLocation = SphereCenter + RandomVec;
	FVector ToEndLocation = EndLocation - TraceStart;
	FVector WithSpread = FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size());

	/*
	DrawDebugSphere(GetWorld(), SphereCenter, SpreadRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLocation, 4.f, 12, FColor::Orange, true);
	DrawDebugLine(GetWorld(), TraceStart, WithSpread, FColor::Cyan, true);
	*/

	return WithSpread;
}

void AHitscanWeapon::TraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHitResult)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FVector End = bUseSpread ?
		TraceWithSpread(TraceStart, HitTarget) :
		TraceStart + (HitTarget - TraceStart) * 1.25f;
	World->LineTraceSingleByChannel(OutHitResult, TraceStart, End, ECollisionChannel::ECC_Visibility);

	FVector BeamEnd = End;
	if (OutHitResult.bBlockingHit)
	{
		BeamEnd = OutHitResult.ImpactPoint;
	}
	if (SmokeBeamParticles)
	{
		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, SmokeBeamParticles, TraceStart, FRotator::ZeroRotator, true);
		if (Beam) Beam->SetVectorParameter(FName("Target"), BeamEnd);
	}
}