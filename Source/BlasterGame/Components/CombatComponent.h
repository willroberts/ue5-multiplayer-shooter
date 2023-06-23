// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "BlasterGame/BlasterTypes/CombatState.h"
#include "BlasterGame/HUD/BlasterHUD.h"
#include "BlasterGame/Weapon/WeaponTypes.h"

#include "CombatComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTERGAME_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void EquipWeapon(class AWeapon* Weapon);
	void UnequipWeapon();
	void FireButtonPressed(bool bPressed);
	void Reload();
	void EndShotgunReload();
	void AddAmmo(EWeaponType AmmoType, int32 AmmoAmount);

	bool bIsLocallyReloading = false;

	FORCEINLINE int32 GetGrenades() const { return Grenades; }

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bAiming);
	void ThrowGrenade();
	void StartGrenadeThrowTimer();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UFUNCTION()
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void ReleaseGrenade(); // Spawn projectile at apex of throw animation.

	UFUNCTION(Server, Reliable)
	void ServerReleaseGrenade(const FVector_NetQuantize& Target);

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireWeapon();
	void FireHitscanWeapon();
	void FireProjectileWeapon();
	void FireMultishotWeapon();
	void FireBurstWeapon();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void LocalMultiFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerMultiFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastMultiFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void TraceUnderCrosshair(FHitResult& TraceHitResult);
	void SetHUDCrosshair(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();
	int32 AmountToReload();

	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachActorToRightHand(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayWeaponEquipSound();
	void ReloadEmptyWeapon();

	/*
	* Called from Animation Blueprints.
	*/

	UFUNCTION(BlueprintCallable)
	void ReloadShotgunShell();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	UPROPERTY()
	class ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_IsAiming)
	bool bIsAiming = false;

	bool bAimButtonPressed = false;

	UFUNCTION()
	void OnRep_IsAiming();

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/*
	* HUD and Crosshair
	*/

	FHUDPackage HUDPackage;
	float CrosshairVelocityFactor;
	float CrosshairFallingFactor;
	float CrosshairAimFactor;
	float CrosshairFiringFactor;
	FVector HitTarget;

	void UpdateWeaponHUD();

	/*
	* Aiming and FOV
	*/

	float DefaultFOV;
	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 50.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/*
	* Weapon firing
	*/

	FTimerHandle FireTimer;
	FTimerHandle BurstFireTimer;
	bool bCanFire = true;
	void StartFireTimer();
	void FireTimerFinished();
	void StartBurstFireTimer();
	void BurstFireTimerFinished();
	bool CanFire();

	FTimerHandle AutoReloadTimer;
	void ScheduleAutoReload();
	void AutoReloadTimerFinished();

	UPROPERTY(EditAnywhere, Category = Combat)
	float AutoReloadAfter = 0.75f; // Seconds

	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo; // For current weapon.

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;
	void InitializeCarriedAmmo();

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingMarksmanAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingStealthRifleAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingScoutRifleAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingRevolverAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingBullpupRifleAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingLightMachineGunAmmo;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	FTimerHandle ReloadTimer;
	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();

	/*
	* Grenades
	*/

	FTimerHandle GrenadeThrowTimer;
	float GrenadeThrowDuration = 1.0f; // 1.5s with 1.5 play rate.
	void ShowAttachedGrenade(bool bShow);
	void UpdateHUDGrenades();

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 3;

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 3;

	UFUNCTION()
	void OnRep_Grenades();
};