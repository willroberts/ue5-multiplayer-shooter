// (c) 2023 Will Roberts

#include "Projectile.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

#include "BlasterGame/BlasterGame.h"
#include "BlasterGame/Character/BlasterCharacter.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Configure collision.
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Register hit detection delegates.
	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}

	// Spawn tracers.
	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,       // Particle effect.
			CollisionBox, // Attachment.
			FName(),      // Not attaching to bones; use empty FName.
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}
}

// OnHit is called on the server when the collision box detects a hit.
void AProjectile::OnHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
) {
	// Trigger impact FX on the character.
	bool bHitPlayer = false;
	ABlasterCharacter* HitPlayer = Cast<ABlasterCharacter>(OtherActor);
	if (HitPlayer)
	{
		bHitPlayer = true;
	}

	// Multicast projectile impact FX.
	MulticastHitFX(Hit.ImpactPoint, bHitPlayer);

	// Destroy the projectile.
	Destroy();
}

void AProjectile::DealAreaDamage()
{
	if (HasAuthority())
	{
		APawn* FiringPawn = GetInstigator();
		if (FiringPawn)
		{
			AController* FiringController = FiringPawn->GetController();
			if (FiringController)
			{
				UGameplayStatics::ApplyRadialDamageWithFalloff(
					this,
					Damage,             // Maximum damage.
					Damage * 0.5,       // Minimum damage.
					GetActorLocation(),
					DamageRadius * 0.5, // Inner radius.
					DamageRadius,       // Outer radius.
					1.f,                // Linear damage falloff.
					UDamageType::StaticClass(),
					TArray<AActor*>(),
					this,
					FiringController
				);
			}
		}
	}
}

// Called when a non-Player hit is detected.
void AProjectile::MulticastHitFX_Implementation(FVector_NetQuantize HitLocation, bool bHitPlayer)
{
	if (bHitPlayer)
	{
		if (PlayerImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PlayerImpactParticles, HitLocation);
		}
		if (PlayerImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, PlayerImpactSound, HitLocation);
		}
	}
	else
	{
		if (SolidImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SolidImpactParticles, HitLocation);
		}
		if (SolidImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, SolidImpactSound, HitLocation);
		}
	}
}

void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(), // No specific attachment point.
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false // Don't automatically destroy.
		);
	}
}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AProjectile::DestroyTimerFinished, DestroyTime);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}