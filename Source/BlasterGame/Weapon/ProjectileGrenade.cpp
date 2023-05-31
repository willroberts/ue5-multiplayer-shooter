// (c) 2023 Will Roberts

#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileGrenade::AProjectileGrenade()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay(); // Bypass AProjectile::BeginPlay().

	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);

	SpawnTrailSystem();
	StartDestroyTimer();
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
	}
}

// Called when the projectile explodes without hitting a player.
void AProjectileGrenade::Destroyed()
{
	// Apply damage.
	DealAreaDamage();

	// Play custom impact FX + sound.
	if (SolidImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SolidImpactParticles, GetActorLocation());
	}
	if (SolidImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SolidImpactSound, GetActorLocation());
	}

	Super::Destroyed();
}