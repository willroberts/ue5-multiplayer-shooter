// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"

#include "BlasterGame/BlasterTypes/TurningInPlace.h"
#include "BlasterGame/Interfaces/CrosshairInteractionInterface.h"
#include "BlasterGame/Weapon/Weapon.h"

#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTERGAME_API ABlasterCharacter : public ACharacter, public ICrosshairInteractionInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	AWeapon* GetEquippedWeapon();
	void PlayFireMontage(bool bAiming);
	void PlayEliminatedMontage();
	FVector GetHitTarget() const;
	virtual void OnRep_ReplicatedMovement() override;
	void Eliminated();
	virtual void Destroyed() override;
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; };
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; };
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; };
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; };
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; };
	FORCEINLINE bool IsEliminated() const { return bEliminated; };
	FORCEINLINE float GetHealth() const { return Health; };
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; };

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminated();

protected:
	virtual void BeginPlay() override;
	virtual void Jump() override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void ReloadButtonPressed();
	void CalculateAO_Pitch();
	float CalculateSpeed();
	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	void FireButtonPressed();
	void FireButtonReleased();
	void DropButtonPressed();
	void PlayHitReactMontage();
	void UpdateHUDHealth();
	void PollPlayerState();

	UFUNCTION()
	void ReceiveDamage(
		AActor* DamagedActor,
		float Damage,
		const UDamageType* DamageType,
		class AController* InstigatorController,
		AActor* DamageCauser
	);

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	// RepNotify function for weapon pickups.
	// Only called on replication; not executed on the server.
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	// Server RPC.
	// Reliable: Guaranteed to be executed, can be retried.
	// Unreliable: Can be dropped.
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerDropButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* EliminatedMontage;

	UPROPERTY(EditAnywhere)
	float CameraHideMeshThreshold = 200.f;
	void CameraHideMesh();

	bool bRotateRootBone;
	float TurnInPlaceThreshold = 0.5f;
	FRotator ProxyRotation;
	FRotator ProxyRotationLastFrame;
	float ProxyYaw;
	float TimeSinceLastMovementRep;

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;

	/*
	* Player health, eliminations, and respawns
	*/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	bool bEliminated = false;
	FTimerHandle RespawnTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Elimination")
	float RespawnDelay = 3.f;
	void RespawnTimerFinished();

	/*
	* Dissolve VFX
	*/

	UPROPERTY(VisibleAnywhere, Category = "Elimination")
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere, Category = "Elimination")
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveMagnitude);
	void StartDissolve();

	UPROPERTY(EditAnywhere, Category = "Elimination")
	UMaterialInstance* DissolveMaterialInstance;

	UPROPERTY(VisibleAnywhere, Category = "Elimination")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	/*
	* Respawn bot
	*/

	UPROPERTY(EditAnywhere)
	class UParticleSystem* RespawnBotEffect;

	UPROPERTY(VisibleAnywhere)
	class UParticleSystemComponent* RespawnBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* RespawnBotSound;
};