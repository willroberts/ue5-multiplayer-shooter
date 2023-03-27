// © 2023 Will Roberts

#include "ShellCasing.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AShellCasing::AShellCasing()
{
	PrimaryActorTick.bCanEverTick = false;
	ShellEjectionImpulse = 10.f;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true); // Simulation Generates Hit Events.
}

void AShellCasing::BeginPlay()
{
	Super::BeginPlay();
	
	CasingMesh->OnComponentHit.AddDynamic(this, &AShellCasing::OnHit);

	// Project the casing away from the weapon mesh.
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);

	// Automatically destroy shell casings after 10 seconds.
	SetLifeSpan(10.f);
}

void AShellCasing::OnHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
) {
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}

	// Prevent further collision events and sounds for this shell casing.
	CasingMesh->SetNotifyRigidBodyCollision(false);
}