// (c) 2023 Will Roberts

#include "BlasterPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include "BlasterGame/Character/BlasterCharacter.h"
#include "BlasterGame/PlayerController/BlasterPlayerController.h"

ABlasterPlayerState::ABlasterPlayerState()
{
	// Configure replication.
	NetUpdateFrequency = 10.f; // 100ms
	MinNetUpdateFrequency = 4.f; // 250ms
}

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
	DOREPLIFETIME(ABlasterPlayerState, EliminationPopupText);
}

// AddToScore runs on the server authority to increment player score, which is replicated.
void ABlasterPlayerState::AddToScore(float Value)
{
	SetScore(GetScore() + Value); // Triggers replication.

	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

// OnRep_Score runs on clients when score is incremented.
void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

// AddToDefeats runs on the server authority to increment player defeats, which is replicated.
void ABlasterPlayerState::AddToDefeats(int32 Value)
{
	Defeats += Value; // Triggers replication.

	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

// OnRep_Defeats runs on clients when defeats is incremented.
void ABlasterPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

// SetEliminationPopup runs on the server authority to display elimination messages, which are replicated.
void ABlasterPlayerState::SetEliminationPopup(FString Message)
{
	EliminationPopupText = Message; // Triggers replication.
	if (EliminationPopupText == "")
	{
		return;
	}

	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->ShowEliminationPopup(EliminationPopupText);
			StartPopupTimer();
		}
	}
}

// OnRep_EliminationPopup runs on clients when elimination messages are set.
void ABlasterPlayerState::OnRep_EliminationPopup()
{
	if (EliminationPopupText == "")
	{
		return;
	}

	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->ShowEliminationPopup(EliminationPopupText);
			StartPopupTimer();
		}
	}
}

void ABlasterPlayerState::StartPopupTimer()
{
	GetWorldTimerManager().SetTimer(EliminationPopupTimer, this, &ABlasterPlayerState::PopupTimerFinished, 3.0f);
}

void ABlasterPlayerState::PopupTimerFinished()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			EliminationPopupText = "";
			Controller->HideEliminationPopup();
		}
	}
}