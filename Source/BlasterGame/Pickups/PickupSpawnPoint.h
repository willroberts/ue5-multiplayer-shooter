// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class BLASTERGAME_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	void SpawnPickup();

	UFUNCTION()
	void StartSpawnTimer(AActor* DestroyedActor);

	void SpawnTimerFinished();

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> PickupClasses;

	UPROPERTY()
	APickup* SpawnedPickup;

private:
	FTimerHandle SpawnTimer;

	UPROPERTY(EditAnywhere)
	float SpawnTimeMin = 30.f;

	UPROPERTY(EditAnywhere)
	float SpawnTimeMax = 30.f;
};