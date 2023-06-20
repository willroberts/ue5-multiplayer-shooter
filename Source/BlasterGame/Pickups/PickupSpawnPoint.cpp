// (c) 2023 Will Roberts

#include "PickupSpawnPoint.h"
#include "Pickup.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnTimer((AActor*)nullptr);
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APickupSpawnPoint::SpawnPickup()
{
	if (PickupClasses.Num() <= 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// Spawn a pickup.
	int32 Index = FMath::RandRange(0, PickupClasses.Num() - 1);
	SpawnedPickup = World->SpawnActor<APickup>(PickupClasses[Index], GetActorTransform());

	// Automatically start the spawn timer when the pickup is destroyed.
	if (HasAuthority() && SpawnedPickup)
	{
		SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnTimer);
	}
}

void APickupSpawnPoint::StartSpawnTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnTimeMin, SpawnTimeMax);
	GetWorldTimerManager().SetTimer(SpawnTimer, this, &APickupSpawnPoint::SpawnTimerFinished, SpawnTime);
}

void APickupSpawnPoint::SpawnTimerFinished()
{
	if (HasAuthority()) SpawnPickup();
}