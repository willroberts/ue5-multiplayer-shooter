// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

UCLASS()
class BLASTERGAME_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();

	virtual void Tick(float DeltaTime);

	virtual void PlayerEliminated(
		class ABlasterCharacter* EliminatedCharacter,
		class ABlasterPlayerController* EliminatedController,
		class ABlasterPlayerController* AttackingController
	);

	virtual void RequestRespawn(ACharacter* Character, AController* Controller);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 30.f;

	float LevelStartTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;
};