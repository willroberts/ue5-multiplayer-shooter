// (c) 2023 Will Roberts

#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// OnHit is called on the server when the collision box detects a hit.
void AProjectileRocket::OnHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
) {
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

	// Super version triggers hit FX before destroying the projectile.
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}