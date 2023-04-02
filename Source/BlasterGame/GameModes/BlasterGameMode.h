// © 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

UCLASS()
class BLASTERGAME_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(
		class ABlasterCharacter* EliminatedCharacter,
		class ABlasterPlayerController* EliminatedController,
		class ABlasterPlayerController* AttackingController
	);
};