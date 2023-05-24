// (c) 2023 Will Roberts

#include "ProjectileRocket.h"

#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	// Register hit detection delegates for clients (Parent class covers server).
	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

	if (RocketTrailSystem)
	{
		RocketTrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			RocketTrailSystem,
			GetRootComponent(),
			FName(), // No specific attachment point.
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false // Don't automatically destroy.
		);
	}

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
					Damage,       // Maximum damage.
					Damage * 0.5, // Minimum damage.
					GetActorLocation(),
					200.f,        // Inner radius.
					500.f,        // Outer radius.
					1.f,          // Linear damage falloff.
					UDamageType::StaticClass(),
					TArray<AActor*>(),
					this,
					FiringController
				);
			}
		}
	}

	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AProjectileRocket::DestroyTimerFinished, DestroyTime);

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
	if (RocketMesh) RocketMesh->SetVisibility(false);

	// Disable collision.
	if (CollisionBox) CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Disable rocket trail.
	if (RocketTrailComponent && RocketTrailComponent->GetSystemInstance())
	{
		RocketTrailComponent->GetSystemInstance()->Deactivate();
	}

	// Stop playing sounds.
	if (RocketFlightSoundComponent && RocketFlightSoundComponent->IsPlaying())
	{
		RocketFlightSoundComponent->Stop();
	}

	// Super version triggers hit FX before destroying the projectile.
	//Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}

void AProjectileRocket::Destroyed()
{
	// Super::Destroyed();
}