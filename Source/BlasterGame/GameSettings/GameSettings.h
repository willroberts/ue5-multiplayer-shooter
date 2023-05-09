// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "GameSettings.generated.h"

UCLASS(Config=Engine)
class BLASTERGAME_API UGameSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UGameSettings();

	/* Game Settings */

	UPROPERTY(EditAnywhere, Config, Category = BlasterGame, meta = (ConsoleVariable = "r.DynamicGlobalIlluminationMethod"))
	bool bLumenEnabled = true;

	UPROPERTY(EditAnywhere, Config, Category = BlasterGame, meta = (ConsoleVariable = "r.Nanite"))
	bool bNaniteEnabled = true;

	UPROPERTY(EditAnywhere, Config, Category = BlasterGame, meta = (ConsoleVariable = "r.Shadow.Virtual.Enable"))
	bool bVirtualShadowMapsEnabled = true;

	UPROPERTY(EditAnywhere, Config, Category = BlasterGame)
	float GlobalVolumeMultiplier = 0.50f;
};