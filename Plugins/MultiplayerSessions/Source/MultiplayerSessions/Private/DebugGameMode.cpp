// (c) 2023 Will Roberts

#include "DebugGameMode.h"
#include "Logger.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

/*************
Public Methods
*************/

// PostLogin overrides the corresponding base class function to log player names and counts.
void ADebugGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
	if (!PlayerState)
	{
		Logger::Log(TEXT("PostLogin: Failed to get PlayerState"), true);
		return;
	}
	FString PlayerName = PlayerState->GetPlayerName();
	Logger::Log(FString::Printf(TEXT("Player %s has joined"), *PlayerName), false);

	if (!GameState)
	{
		Logger::Log(TEXT("PostLogin: Failed to get GameState"), true);
		return;
	}
	int32 NumPlayers = GameState.Get()->PlayerArray.Num();
	Logger::Log(FString::Printf(TEXT("Players in game: %d"), NumPlayers), false);
}

// Logout overrides the corresponding base class function to log player names and counts.
void ADebugGameMode::Logout(AController* ExitingPlayer)
{
	Super::Logout(ExitingPlayer);

	APlayerState* PlayerState = ExitingPlayer->GetPlayerState<APlayerState>();
	if (!PlayerState)
	{
		Logger::Log(TEXT("Logout: Failed to get PlayerState"), true);
		return;
	}
	FString PlayerName = PlayerState->GetPlayerName();
	Logger::Log(FString::Printf(TEXT("Player %s has disconnected"), *PlayerName), false);

	if (!GameState)
	{
		Logger::Log(TEXT("Logout: Failed to get GameState"), true);
		return;
	}
	int32 NumPlayers = GameState.Get()->PlayerArray.Num();
	Logger::Log(FString::Printf(TEXT("Players in game: %d"), NumPlayers - 1), false);
}