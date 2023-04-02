// © 2023 Will Roberts

#include "BlasterGameMode.h"

#include "BlasterGame/Character/BlasterCharacter.h"
#include "BlasterGame/PlayerController/BlasterPlayerController.h"

void ABlasterGameMode::PlayerEliminated(
	class ABlasterCharacter* EliminatedCharacter,
	class ABlasterPlayerController* EliminatedController,
	class ABlasterPlayerController* AttackingController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Eliminated();
	}
}