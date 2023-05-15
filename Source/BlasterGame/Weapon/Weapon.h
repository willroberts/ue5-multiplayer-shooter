// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "Default Maximum Value")
};

UCLASS()
class BLASTERGAME_API AWeapon : public AActor
{
	GENERATED_BODY()

public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	void ShowPickupWidget(bool bShowWidget);
	void SetWeaponState(EWeaponState State);
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	FORCEINLINE class USphereComponent* GetAreaSphere() const { return AreaSphere; };
	FORCEINLINE class USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; };
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; };
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; };

	/*
	* Textures for weapon crosshair.
	*/

	UPROPERTY(EditAnywhere, Category = Crosshair)
	class UTexture2D* CrosshairCenter;

	UPROPERTY(EditAnywhere, Category = Crosshair)
	UTexture2D* CrosshairTop;

	UPROPERTY(EditAnywhere, Category = Crosshair)
	UTexture2D* CrosshairBottom;

	UPROPERTY(EditAnywhere, Category = Crosshair)
	UTexture2D* CrosshairLeft;

	UPROPERTY(EditAnywhere, Category = Crosshair)
	UTexture2D* CrosshairRight;

	/*
	* Weapon fire modes
	*/

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float FireInterval = 0.125f; // 480 RPM.

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bAutomaticFireMode = false;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

private:	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AShellCasing> CasingClass;

	/*
	* Zoomed FOV while aiming
	*/

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 50.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	/*
	* Ammo
	*/

	UPROPERTY()
	class ABlasterCharacter* OwnerCharacter;

	UPROPERTY()
	class ABlasterPlayerController* OwnerController;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	void ConsumeAmmo();

	UFUNCTION()
	void OnRep_Ammo();

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;
};