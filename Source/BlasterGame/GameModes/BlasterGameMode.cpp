// © 2023 Will Roberts

#include "BlasterGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

#include "BlasterGame/Character/BlasterCharacter.h"
#include "BlasterGame/PlayerController/BlasterPlayerController.h"

void ABlasterGameMode::PlayerEliminated(
	class ABlasterCharacter* EliminatedCharacter,
	class ABlasterPlayerController* EliminatedController,
	class ABlasterPlayerController* AttackingController
)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Eliminated();
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* Character, AController* Controller)
{
	// Destroy the eliminated character.
	if (Character)
	{
		Character->Reset(); // Detach controller.
		Character->Destroy();
	}

	// Reassign the controller to a new character.
	if (Controller)
	{
		// Warning: Poor performance scaling with large quantities.
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

		// Choose a random spawn point.
		int32 RandomIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(Controller, PlayerStarts[RandomIndex]);
	}
}