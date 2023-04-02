// © 2023 Will Roberts

#include "BlasterPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

#include "BlasterGame/HUD/BlasterHUD.h"
#include "BlasterGame/HUD/CharacterOverlay.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	// Initialize BlasterHUD if necessary.
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