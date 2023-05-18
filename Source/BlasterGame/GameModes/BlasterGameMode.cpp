// (c) 2023 Will Roberts

#include "BlasterGameMode.h"

#include "Components/AudioComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

#include "BlasterGame/Character/BlasterCharacter.h"
#include "BlasterGame/GameState/BlasterGameState.h"
#include "BlasterGame/PlayerController/BlasterPlayerController.h"
#include "BlasterGame/PlayerState/BlasterPlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartTime;
		if (CountdownTime <= 0.f)
		{
			if (MusicComponent)
			{
				MusicComponent->Stop();
				MusicComponent = nullptr;
			}
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartTime;
		if (CountdownTime <= 0.f)
		{
			if (GameMusic)
			{
				MusicComponent = UGameplayStatics::SpawnSound2D(this, GameMusic, 0.25);
			}
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = WarmupTime + MatchTime + CooldownTime - GetWorld()->GetTimeSeconds() + LevelStartTime;
		if (CountdownTime <= 0.f)
		{
			if (MusicComponent)
			{
				MusicComponent->Stop();
				MusicComponent = nullptr;
			}
			RestartGame();
		}
	}
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartTime = GetWorld()->GetTimeSeconds();
	if (ElevatorMusic)
	{
		MusicComponent = UGameplayStatics::SpawnSound2D(this, ElevatorMusic, 0.25);
	}
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator Iter = GetWorld()->GetPlayerControllerIterator(); Iter; Iter++)
	{
		ABlasterPlayerController* Controller = Cast<ABlasterPlayerController>(*Iter);
		if (Controller)
		{
			Controller->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::PlayerEliminated(
	class ABlasterCharacter* EliminatedCharacter,
	class ABlasterPlayerController* EliminatedController,
	class ABlasterPlayerController* AttackingController
)
{
	ABlasterPlayerState* AttackingPlayerState = AttackingController ? Cast<ABlasterPlayerState>(AttackingController->PlayerState) : nullptr;
	ABlasterPlayerState* EliminatedPlayerState = EliminatedController ? Cast<ABlasterPlayerState>(EliminatedController->PlayerState) : nullptr;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	// Increment score for the attacking player if they did not eliminate themselves.
	// Decrement score for the attacking player if they did.
	if (AttackingPlayerState)
	{
		if (AttackingPlayerState != EliminatedPlayerState)
		{
			AttackingPlayerState->AddToScore(1.f);
			if (BlasterGameState)
			{
				BlasterGameState->UpdatePlayerScore(AttackingPlayerState);
			}
		}
		else
		{
			AttackingPlayerState->AddToScore(-1.f);
		}
	}

	// Increment defeats for the eliminated player.
	if (EliminatedPlayerState)
	{
		EliminatedPlayerState->AddToDefeats(1);
	}

	// Show timed popup messages for eliminations.
	if (AttackingPlayerState && EliminatedPlayerState)
	{
		AttackingPlayerState->SetEliminationPopup(FString::Printf(TEXT("You eliminated %s!"), *EliminatedPlayerState->GetPlayerName()));
		EliminatedPlayerState->SetEliminationPopup(FString::Printf(TEXT("You were eliminated by %s!"), *AttackingPlayerState->GetPlayerName()));
	}

	// Call the elimination method to drop equipped weapons, play animations, and respawn the character.
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