// (c) 2023 Will Roberts

#include "BlasterPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "BlasterGame/Character/BlasterCharacter.h"
#include "BlasterGame/GameModes/BlasterGameMode.h"
#include "BlasterGame/HUD/AnnouncementWidget.h"
#include "BlasterGame/HUD/BlasterHUD.h"
#include "BlasterGame/HUD/CharacterOverlay.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	ServerCheckMatchState();
}

void ABlasterPlayerController::PollOverlayState()
{
	// Ensure health, score, and defeats are set.
	if (!CharacterOverlay && BlasterHUD && BlasterHUD->CharacterOverlay)
	{
		CharacterOverlay = BlasterHUD->CharacterOverlay;
		SetHUDHealth(HUDHealth, HUDMaxHealth);
		SetHUDScore(HUDScore);
		SetHUDDefeats(HUDDefeats);
	}

	// Ensure level start time is set in the case where Controller::BeginPlay runs before GameMode::BeginPlay.
	if (LevelStartTime == 0.f)
	{
		ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
		if (GameMode)
		{
			LevelStartTime = GameMode->LevelStartTime;
		}
	}
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PollOverlayState();
	SyncTime(DeltaTime);
	SetHUDTime();
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

void ABlasterPlayerController::SyncTime(float DeltaTime)
{
	TimeSinceLastSync += DeltaTime;
	if (IsLocalController() && TimeSinceLastSync > TimeSyncFreq)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSinceLastSync = 0.f;
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		MatchState = GameMode->GetMatchState();
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartTime = GameMode->LevelStartTime;

		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartTime);

		if (BlasterHUD && MatchState == MatchState::WaitingToStart)
		{
			BlasterHUD->AddAnnouncement();
		}
	}
}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName State, float WTime, float MTime, float StartTime)
{
	MatchState = State;
	WarmupTime = WTime;
	MatchTime = MTime;
	LevelStartTime = StartTime;

	OnMatchStateSet(MatchState);
	if (BlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncement();
	}
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController()) {
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar &&
		BlasterHUD->CharacterOverlay->HealthText)
	{
		const float HealthPct = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPct);
		FString HealthText = FString::Printf(
			TEXT("%d/%d"),
			FMath::CeilToInt(Health),
			FMath::CeilToInt(MaxHealth)
		);
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
		bInitCharacterOverlay = true;
	}
}

void ABlasterPlayerController::SetHUDMatchTimer(float Time)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchTimerText)
	{
		int32 Minutes = FMath::FloorToInt(Time / 60.f);
		int32 Seconds = Time - (Minutes * 60);

		FString TimerText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchTimerText->SetText(FText::FromString(TimerText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementTimer(float Time)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->AnnouncementWidget &&
		BlasterHUD->AnnouncementWidget->MatchTimerText)
	{
		int32 Minutes = FMath::FloorToInt(Time / 60.f);
		int32 Seconds = Time - (Minutes * 60);

		FString TimerText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->AnnouncementWidget->MatchTimerText->SetText(FText::FromString(TimerText));
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreValueText)
	{
		FString ScoreText = FString::Printf(TEXT("%d"),	FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreValueText->SetText(FText::FromString(ScoreText));
	}
	else
	{
		HUDScore = Score;
		bInitCharacterOverlay = true;
	}
}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DefeatsValueText)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsValueText->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		HUDDefeats = Defeats;
		bInitCharacterOverlay = true;
	}
}

void ABlasterPlayerController::SetHUDWeaponType(EWeaponType WeaponType)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponTypeText)
	{
		switch (WeaponType)
		{
		case EWeaponType::EWT_AssaultRifle:
			BlasterHUD->CharacterOverlay->WeaponTypeText->SetText(FText::FromString("Assault Rifle"));
			break;
		default:
			BlasterHUD->CharacterOverlay->WeaponTypeText->SetText(FText::FromString("Unarmed"));
			break;
		}
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoValueText)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoValueText->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoValueText)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoValueText->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::ShowEliminationPopup(FString Message)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->EliminationPopupText)
	{
		BlasterHUD->CharacterOverlay->EliminationPopupText->SetText(FText::FromString(Message));
		BlasterHUD->CharacterOverlay->EliminationPopupText->SetColorAndOpacity(Message.Contains(TEXT("eliminated by")) ? FLinearColor::Red : FLinearColor::White);
		BlasterHUD->CharacterOverlay->EliminationPopupText->SetVisibility(ESlateVisibility::Visible);
	}
}

void ABlasterPlayerController::HideEliminationPopup()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->EliminationPopupText)
	{
		BlasterHUD->CharacterOverlay->EliminationPopupText->SetText(FText::FromString(""));
		BlasterHUD->CharacterOverlay->EliminationPopupText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartTime;
	}

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart) SetHUDAnnouncementTimer(TimeLeft);
		else if (MatchState == MatchState::InProgress) SetHUDMatchTimer(TimeLeft);
	}

	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float SendTime)
{
	float RecvTime = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(SendTime, RecvTime);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float SendTime, float RecvTime)
{
	float RTT = GetWorld()->GetTimeSeconds() - SendTime;
	float CurrentServerTime = RecvTime + (RTT * 0.5f);
	ClientServerTimeDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	return GetWorld()->GetTimeSeconds() + ClientServerTimeDelta;
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchStart();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchStart();
	}
}

void ABlasterPlayerController::HandleMatchStart()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->AddCharacterOverlay();
		if (BlasterHUD->AnnouncementWidget)
		{
			BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}