// © 2023 Will Roberts

#include "BlasterPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

#include "BlasterGame/Character/BlasterCharacter.h"
#include "BlasterGame/HUD/BlasterHUD.h"
#include "BlasterGame/HUD/CharacterOverlay.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
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
}

void ABlasterPlayerController::ShowEliminationPopup(FString Message)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->EliminationPopupText)
	{
		// Use red color for "eliminated by" messages.
		bool bUseRed = Message.Contains(TEXT("eliminated by"));

		BlasterHUD->CharacterOverlay->EliminationPopupText->SetText(FText::FromString(Message));
		BlasterHUD->CharacterOverlay->EliminationPopupText->SetColorAndOpacity(bUseRed ? FLinearColor::Red : FLinearColor::White);
		BlasterHUD->CharacterOverlay->EliminationPopupText->SetVisibility(ESlateVisibility::Visible);

		// Clear the popup after a few seconds.
		StartPopupTimer();
	}
}

void ABlasterPlayerController::HideEliminationPopup()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->EliminationPopupText)
	{
		BlasterHUD->CharacterOverlay->EliminationPopupText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ABlasterPlayerController::StartPopupTimer()
{
	GetWorldTimerManager().SetTimer(EliminationPopupTimer, this, &ABlasterPlayerController::PopupTimerFinished, 3.0f);
}

void ABlasterPlayerController::PopupTimerFinished()
{
	HideEliminationPopup();
}