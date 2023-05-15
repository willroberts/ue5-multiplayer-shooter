// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

UCLASS()
class BLASTERGAME_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreValueText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsValueText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EliminationPopupText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoValueText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoValueText;
};