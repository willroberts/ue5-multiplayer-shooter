// © 2023 Will Roberts

#include "BlasterGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

#include "BlasterGame/Character/BlasterCharacter.h"
#include "BlasterGame/PlayerController/BlasterPlayerController.h"
#include "BlasterGame/PlayerState/BlasterPlayerState.h"

void ABlasterGameMode::PlayerEliminated(
	class ABlasterCharacter* EliminatedCharacter,
	class ABlasterPlayerController* EliminatedController,
	class ABlasterPlayerController* AttackingController
)
{
	ABlasterPlayerState* AttackingPlayerState = AttackingController ? Cast<ABlasterPlayerState>(AttackingController->PlayerState) : nullptr;
	ABlasterPlayerState* EliminatedPlayerState = EliminatedController ? Cast<ABlasterPlayerState>(EliminatedController->PlayerState) : nullptr;

	if (AttackingPlayerState && AttackingPlayerState != EliminatedPlayerState)
	{
		AttackingPlayerState->AddToScore(1.f);
	}

	if (EliminatedPlayerState)
	{
		EliminatedPlayerState->AddToDefeats(1);
	}

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