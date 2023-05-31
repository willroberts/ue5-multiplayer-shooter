// (c) 2023 Will Roberts

#include "ProjectileRocket.h"
#include "RocketMovementComponent.h"

#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Located in parent class (Projectile.h).
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);

	// Uncomment to use custom movement component.
	//RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	//RocketMovementComponent->bRotationFollowsVelocity = true;
	//RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	// Register hit detection delegates for clients (Parent class covers server).
	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

	SpawnTrailSystem();

	if (RocketFlightSoundCue && RocketFlightSoundAttenuation)
	{
		RocketFlightSoundComponent = UGameplayStatics::SpawnSoundAttached(
			RocketFlightSoundCue,
			GetRootComponent(),
			FName(), // No specific attachment point.
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,   // Don't stop when actor is destroyed.
			1.0f,    // Volume.
			1.0f,    // Pitch.
			0.0f,    // Start time.
			RocketFlightSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false    // Don't automatically destroy.
		);
	}
}

// OnHit is called on the server when the collision box detects a hit.
void AProjectileRocket::OnHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
) {
	// Enable the following code to prevent the rocket from hitting its own instigator.
	// NOTE: Destruction will not work as a result, so the ProjectileRocket would need a custom
	// ProjectileMovementComponent in order to work properly (see constructor).
	/*
	if (OtherActor == GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("Rocket hit instigator; disregarding hit"));
		return;
	}
	*/

	DealAreaDamage();
	StartDestroyTimer();

	// Play impact FX.
	if (PlayerImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PlayerImpactParticles, GetActorTransform());
	}
	if (PlayerImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PlayerImpactSound, GetActorLocation());
	}

	// Hide the rocket mesh.
	if (ProjectileMesh) ProjectileMesh->SetVisibility(false);

	// Disable collision.
	if (CollisionBox) CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Disable rocket trail.
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstance())
	{
		TrailSystemComponent->GetSystemInstance()->Deactivate();
	}

	// Stop playing sounds.
	if (RocketFlightSoundComponent && RocketFlightSoundComponent->IsPlaying())
	{
		RocketFlightSoundComponent->Stop();
	}

	// Super version triggers hit FX before destroying the projectile.
	//Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileRocket::Destroyed()
{
	// Super::Destroyed();
}