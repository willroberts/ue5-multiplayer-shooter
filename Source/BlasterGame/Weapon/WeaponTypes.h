#pragma once

#define TRACE_LENGTH 80000 // 800 meters.

#define OUTLINE_HIGHLIGHT_PURPLE 250
#define OUTLINE_HIGHLIGHT_BLUE 251
#define OUTLINE_HIGHLIGHT_TAN 252

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AutomaticRifle UMETA(DisplayName = "Automatic Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_Submachinegun UMETA(DisplayName = "Submachine Gun"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_MarksmanRifle UMETA(DisplayName = "Marksman Rifle"),
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),

	EWT_MAX UMETA(DisplayName = "Default MAX")
};

UENUM(BlueprintType)
enum class EWeaponRarity : uint8
{
	EWR_Common UMETA(DisplayName = "Common"),
	EWR_Rare UMETA(DisplayName = "Rare"),
	EWR_Legendary UMETA(DisplayName = "Legendary"),

	EWR_MAX UMETA(DisplayName = "Default MAX")
};