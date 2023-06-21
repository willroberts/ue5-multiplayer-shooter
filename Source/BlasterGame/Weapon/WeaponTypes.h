#pragma once

#define TRACE_LENGTH 80000 // 800 meters.

#define OUTLINE_HIGHLIGHT_PURPLE 250
#define OUTLINE_HIGHLIGHT_BLUE 251
#define OUTLINE_HIGHLIGHT_TAN 252

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AutomaticRifle UMETA(DisplayName = "Automatic Rifle"), // M4, AK47
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_Submachinegun UMETA(DisplayName = "Submachine Gun"), // MAC11, P90
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_MarksmanRifle UMETA(DisplayName = "Marksman Rifle"), // G3
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),
	EWT_StealthRifle UMETA(DisplayName = "Stealth Rifle"), // AS VAL
	EWT_ScoutRifle UMETA(DisplayName = "Scout Rifle"),
	EWT_Revolver UMETA(DisplayName = "Revolver"),
	EWT_BullpupRifle UMETA(DisplayName = "Bullpup Rifle"), // AUG
	EWT_LightMachineGun UMETA(DisplayName = "Light Machine Gun"), // M249

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