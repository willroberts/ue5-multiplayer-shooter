// (c) 2023 Will Roberts

#include "BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "BlasterGame/PlayerState/BlasterPlayerState.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
}

void ABlasterGameState::UpdatePlayerScore(class ABlasterPlayerState* Player)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(Player);
		TopScore = Player->GetScore();
	}
	else if (Player->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(Player);
	}
	else if (Player->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.Add(Player);
		TopScore = Player->GetScore();
	}
}