// © 2023 Will Roberts

#include "OverheadWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

//
// Public Methods
//

void UOverheadWidget::SetDisplayText(FString Text)
{
	if (Text == "")
	{
		return;
	}

	DisplayText->SetText(FText::FromString(Text));
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	if (!InPawn)
	{
		// Failed to get Pawn.
		return;
	}

	// Retrieve player name from player state.
	APlayerState* State = InPawn->GetPlayerState();
	if (!State)
	{
		// Failed to get Player State.
		return;
	}
	FString PlayerName = State->GetPlayerName();
	if (PlayerName == "")
	{
		PlayerName = "Unknown";
	}

	// Notes on network roles:
	// ROLE_Authority on hosting client, Proxy on other clients.
	// Always ROLE_Authority on clients, Proxy on servers.
	FString LocalRole = RoleToString(InPawn->GetLocalRole());
	FString RemoteRole = RoleToString(InPawn->GetRemoteRole());
	SetDisplayText(
		FString::Printf(TEXT("Player Name: %s"),
			*PlayerName,
			*LocalRole,
			*RemoteRole
		)
	);
}

//
// Private Methods
//

FString UOverheadWidget::RoleToString(ENetRole Role)
{
	switch (Role)
	{
	case ENetRole::ROLE_Authority:
		return FString("Authority");
	case ENetRole::ROLE_AutonomousProxy:
		return FString("Autonomous Proxy");
	case ENetRole::ROLE_SimulatedProxy:
		return FString("Simulated Proxy");
	case ENetRole::ROLE_None:
		return FString("None");
	default:
		return FString("Unknown");
	}
}