#pragma once

#define TRACE_LENGTH 80000 // 800 meters.

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

	EWT_MAX UMETA(DisplayName = "Default MAX")
};