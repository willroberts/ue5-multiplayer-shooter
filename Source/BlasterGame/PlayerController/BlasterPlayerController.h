// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "BlasterGame/Weapon/WeaponTypes.h"

#include "BlasterPlayerController.generated.h"

UCLASS()
class BLASTERGAME_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void ReceivedPlayer() override;
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDMatchTimer(float Time);
	void SetHUDAnnouncementTimer(float Time);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponType(EWeaponType WeaponType);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDGrenades(int32 Grenades);
	void ShowEliminationPopup(FString Message);
	void HideEliminationPopup();
	virtual float GetServerTime();
	void OnMatchStateSet(FName State);
	void HandleMatchWarmup();
	void HandleMatchStart();
	void HandleMatchCooldown();

protected:
	virtual void BeginPlay() override;
	void PollOverlayState();
	void SetHUDTime();

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float SendTime);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float SendTime, float RecvTime);

	float ClientServerTimeDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFreq = 5.f;

	float TimeSinceLastSync = 0.f;

	void SyncTime(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName State, float WTime, float MTime, float CTime, float StartTime);

private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;
	float LevelStartTime = 0.f; // Seconds since server started the level.
	float WarmupTime = 0.f;
	float MatchTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY()
	class ABlasterGameMode* GameMode;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDShield;
	float HUDMaxShield;
	float HUDScore;
	int32 HUDDefeats;
	int32 HUDWeaponAmmo;
	int32 HUDCarriedAmmo;

	bool bInitializeHealth = false;
	bool bInitializeShield = false;
	bool bInitializeScore = false;
	bool bInitializeGrenades = false;
	bool bInitializeAmmo = false;

	/*
	* Music
	*/

	UPROPERTY(EditAnywhere)
	class USoundCue* ElevatorMusic;

	UPROPERTY(EditAnywhere)
	USoundCue* GameMusic;

	UPROPERTY()
	class UAudioComponent* MusicComponent;
};