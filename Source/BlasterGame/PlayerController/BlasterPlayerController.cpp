// (c) 2023 Will Roberts

#include "BlasterPlayerController.h"

#include "Components/AudioComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"

#include "BlasterGame/Character/BlasterCharacter.h"
#include "BlasterGame/Components/CombatComponent.h"
#include "BlasterGame/GameModes/BlasterGameMode.h"
#include "BlasterGame/GameState/BlasterGameState.h"
#include "BlasterGame/HUD/AnnouncementWidget.h"
#include "BlasterGame/HUD/BlasterHUD.h"
#include "BlasterGame/HUD/CharacterOverlay.h"
#include "BlasterGame/PlayerState/BlasterPlayerState.h"

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
		if (CharacterOverlay)
		{
			if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
			if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
			if (bInitializeScore) SetHUDScore(HUDScore);
			if (bInitializeScore) SetHUDDefeats(HUDDefeats);

			if (bInitializeAmmo)
			{
				SetHUDWeaponAmmo(HUDWeaponAmmo);
				SetHUDCarriedAmmo(HUDCarriedAmmo);
			}

			// Update grenades value.
			ABlasterCharacter* Char = Cast<ABlasterCharacter>(GetPawn());
			if (bInitializeGrenades && Char && Char->GetCombatComponent())
			{
				SetHUDGrenades(Char->GetCombatComponent()->GetGrenades());
			}
		}
	}

	// Ensure level start time is set in the case where Controller::BeginPlay runs before GameMode::BeginPlay.
	if (LevelStartTime == 0.f)
	{
		if (this == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("`this` was nullptr in BlasterPlayerController::PollOverlayState"));
			return;
		}
		GameMode = GameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : GameMode;
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
	if (this == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("`this` was nullptr in BlasterPlayerController::ServerCheckMatchState_"));
		return;
	}
	GameMode = GameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : GameMode;
	if (GameMode)
	{
		MatchState = GameMode->GetMatchState();
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartTime = GameMode->LevelStartTime;

		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartTime);

		if (BlasterHUD && MatchState == MatchState::WaitingToStart)
		{
			BlasterHUD->AddAnnouncement();
		}
	}
}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName State, float WTime, float MTime, float CTime, float StartTime)
{
	MatchState = State;
	WarmupTime = WTime;
	MatchTime = MTime;
	CooldownTime = CTime;
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
		bInitializeHealth = true;
	}
}

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ShieldBar &&
		BlasterHUD->CharacterOverlay->ShieldText)
	{
		const float ShieldPct = Shield / MaxShield;
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPct);
		FString ShieldText = FString::Printf(
			TEXT("%d/%d"),
			FMath::CeilToInt(Shield),
			FMath::CeilToInt(MaxShield)
		);
		BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
		bInitializeShield = true;
	}
}

void ABlasterPlayerController::SetHUDMatchTimer(float Time)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchTimerText)
	{
		if (Time < 0.f)
		{
			BlasterHUD->CharacterOverlay->MatchTimerText->SetText(FText());
			return;
		}

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
		if (Time < 0.f)
		{
			BlasterHUD->AnnouncementWidget->MatchTimerText->SetText(FText());
			return;
		}

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
		bInitializeScore = true;
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
		bInitializeScore = true;
	}
}

void ABlasterPlayerController::SetHUDWeaponType(EWeaponType WeaponType)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponTypeText)
	{
		FText WeaponText;

		switch (WeaponType)
		{
		case EWeaponType::EWT_AutomaticRifle:
			WeaponText = FText::FromString("Automatic Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			WeaponText = FText::FromString("Rocket Launcher");
			break;
		case EWeaponType::EWT_Pistol:
			WeaponText = FText::FromString("Pistol");
			break;
		case EWeaponType::EWT_Submachinegun:
			WeaponText = FText::FromString("Submachine Gun");
			break;
		case EWeaponType::EWT_Shotgun:
			WeaponText = FText::FromString("Shotgun");
			break;
		case EWeaponType::EWT_MarksmanRifle:
			WeaponText = FText::FromString("Marksman Rifle");
			break;
		case EWeaponType::EWT_SniperRifle:
			WeaponText = FText::FromString("Sniper Rifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			WeaponText = FText::FromString("Grenade Launcher");
			break;
		default:
			WeaponText = FText::FromString("Unarmed");
			break;
		}

		BlasterHUD->CharacterOverlay->WeaponTypeText->SetText(WeaponText);
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
	else
	{
		HUDWeaponAmmo = Ammo;
		bInitializeAmmo = true;
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
	else
	{
		HUDCarriedAmmo = Ammo;
		bInitializeAmmo = true;
	}
}

void ABlasterPlayerController::SetHUDGrenades(int32 Grenades)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->GrenadesValueText)
	{
		FString ValueStr = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->GrenadesValueText->SetText(FText::FromString(ValueStr));
	}
	else
	{
		bInitializeGrenades = true;
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
	else if (MatchState == MatchState::Cooldown)
	{
		TimeLeft = WarmupTime + MatchTime + CooldownTime - GetServerTime() + LevelStartTime;
	}

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (HasAuthority())
	{
		if (this == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("`this` was nullptr in BlasterPlayerController::SetHUDTime"));
		}
		else
		{
			GameMode = GameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : GameMode;
			if (GameMode)
			{
				SecondsLeft = FMath::CeilToInt(GameMode->GetCountdownTime() + LevelStartTime);
			}
		}
	}

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementTimer(TimeLeft);
		}
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

	if (MatchState == MatchState::WaitingToStart)
	{
		HandleMatchWarmup();
	}
	else if (MatchState == MatchState::InProgress)
	{
		HandleMatchStart();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleMatchCooldown();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::WaitingToStart)
	{
		HandleMatchWarmup();
	}
	else if (MatchState == MatchState::InProgress)
	{
		HandleMatchStart();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleMatchCooldown();
	}
}

void ABlasterPlayerController::HandleMatchWarmup()
{
	if (MusicComponent)
	{
		MusicComponent->Stop();
		MusicComponent = nullptr;
	}
	if (ElevatorMusic)
	{
		MusicComponent = UGameplayStatics::SpawnSound2D(this, ElevatorMusic, 1.0);
	}
}

void ABlasterPlayerController::HandleMatchStart()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		if (!BlasterHUD->CharacterOverlay) BlasterHUD->AddCharacterOverlay();
		if (BlasterHUD->AnnouncementWidget)
		{
			BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (MusicComponent)
	{
		MusicComponent->Stop();
		MusicComponent = nullptr;
	}
}

void ABlasterPlayerController::HandleMatchCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		if (BlasterHUD->AnnouncementWidget)
		{
			BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Visible);
			if (BlasterHUD->AnnouncementWidget->AnnouncementText)
			{
				FText AnnouncementText = FText::FromString("A new match will begin soon!");
				BlasterHUD->AnnouncementWidget->AnnouncementText->SetText(AnnouncementText);
			}
			if (BlasterHUD->AnnouncementWidget->InfoText)
			{
				FString InfoTextString = FString("");
				ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
				if (BlasterGameState)
				{
					TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;

					if (TopPlayers.Num() == 0)
					{
						InfoTextString = FString("No one wins the round!");
					}
					else if (TopPlayers.Num() == 1)
					{
						ABlasterPlayerState* CurrentPlayer = GetPlayerState<ABlasterPlayerState>();
						if (CurrentPlayer && CurrentPlayer == TopPlayers[0])
						{
							InfoTextString = FString("You win the round!");
						}
						else
						{
							InfoTextString = FString(FString::Printf(TEXT("%s wins the round!"), *TopPlayers[0]->GetPlayerName()));
						}
					}
					else if (TopPlayers.Num() > 1)
					{
						InfoTextString = FString("Multiple players tied the round!\n\n");
						for (auto TiedPlayer : TopPlayers)
						{
							InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
						}
					}
				}
				BlasterHUD->AnnouncementWidget->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}

	ABlasterCharacter* Char = Cast<ABlasterCharacter>(GetPawn());
	if (Char)
	{
		Char->bDisableCombatActions = true;
		if (Char->GetCombatComponent())
		{
			Char->GetCombatComponent()->FireButtonPressed(false);
		}
	}

	if (MusicComponent)
	{
		MusicComponent->Stop();
		MusicComponent = nullptr;
	}
	if (GameMusic)
	{
		MusicComponent = UGameplayStatics::SpawnSound2D(this, GameMusic, 1.0);
	}
}