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
	virtual void OnPossess(APawn* InPawn) override;
	virtual void ReceivedPlayer() override;
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDMatchTimer(float Time);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponType(EWeaponType WeaponType);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void ShowEliminationPopup(FString Message);
	void HideEliminationPopup();
	virtual float GetServerTime();

protected:
	virtual void BeginPlay() override;
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
private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;
	float MatchTime = 120.f;
	uint32 CountdownInt = 0;
};