// (c) 2023 Will Roberts

#include "CombatComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

#include "BlasterGame/Weapon/Weapon.h"
#include "BlasterGame/Character/BlasterAnimInstance.h"
#include "BlasterGame/Character/BlasterCharacter.h"
#include "BlasterGame/PlayerController/BlasterPlayerController.h"

//
// Public Methods
//

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 1000.f;
	AimWalkSpeed = 750.f;
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		SetHUDCrosshair(DeltaTime);

		FHitResult HitResult;
		TraceUnderCrosshair(HitResult);
		HitTarget = HitResult.ImpactPoint;

		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!Character || !WeaponToEquip) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	DropEquippedWeapon();
	EquippedWeapon = WeaponToEquip; // Triggers replication.
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	EquippedWeapon->SetOwner(Character);
	UpdateCarriedAmmo();
	PlayWeaponEquipSound();

	// Update animation bits.
	AttachActorToRightHand(EquippedWeapon);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;

	ReloadEmptyWeapon();
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon) UnequipWeapon();
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (!Character || !Character->GetMesh() || !ActorToAttach || !EquippedWeapon) return;

	bool bUseAltSocket = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
	                     EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Submachinegun;
	FName SocketName = bUseAltSocket ? FName("AltLeftHandSocket") : FName("LeftHandSocket");

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket) HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (!Character || !Character->GetMesh() || !ActorToAttach) return;

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket) HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (!EquippedWeapon) return;

	// Update ammo counts. Map only exists on the server.
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; // Triggers replication.
	}
	if (Controller && Controller->HasAuthority()) UpdateWeaponHUD(); // Need authority check here?
}

void UCombatComponent::PlayWeaponEquipSound()
{
	if (Character && EquippedWeapon && EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EquipSound, Character->GetActorLocation());
	}
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon && EquippedWeapon->IsEmpty()) Reload();
}

void UCombatComponent::UnequipWeapon()
{
	if (!EquippedWeapon) return;

	EquippedWeapon->Dropped();
	EquippedWeapon = nullptr; // Triggers replication.
	if (Controller && Controller->HasAuthority()) UpdateWeaponHUD();
}

// Runs on clients only.
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (!Character) return;

	if (EquippedWeapon)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		PlayWeaponEquipSound();
	}
	
	UpdateWeaponHUD();
}

void UCombatComponent::UpdateWeaponHUD()
{
	if (!Character) return;
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (!Controller) return;

	Controller->SetHUDWeaponType(EquippedWeapon != nullptr ? EquippedWeapon->GetWeaponType() : EWeaponType::EWT_MAX);
	Controller->SetHUDWeaponAmmo(EquippedWeapon != nullptr ? EquippedWeapon->GetAmmo() : 0);
	Controller->SetHUDCarriedAmmo(EquippedWeapon != nullptr ? CarriedAmmo : 0);
}

// Triggered on client input.
void UCombatComponent::Reload()
{
	// Prevent reloading when already reloading.
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	// Prevent reloading when out of ammo to load.
	if (CarriedAmmo <= 0) return;
	// Prevent reloading when weapon is already full.
	if (EquippedWeapon && EquippedWeapon->IsFull()) return;

	ServerReload();
}

// Performs actions needed on both server and clients.
void UCombatComponent::HandleReload()
{
	if (!Character || !EquippedWeapon) return;

	Character->PlayReloadMontage();

	// Call FinishReloading() after reload animation duration.
	// Prevents reloading bugs when the animation notify never happens.
	Character->GetWorldTimerManager().SetTimer(
		ReloadTimer,
		this,
		&UCombatComponent::FinishReloading,
		EquippedWeapon->GetReloadDuration()
	);
}

int32 UCombatComponent::AmountToReload()
{
	if (!EquippedWeapon) return 0;
	if (!CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) return 0;

	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];

	return FMath::Clamp(RoomInMag, 0, FMath::Min(RoomInMag, AmountCarried));
}

// Performs actions needed on server only.
void UCombatComponent::ServerReload_Implementation()
{
	if (!Character || !EquippedWeapon) return;

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

// Called from animation blueprint when adding a shotgun shell during reload.
void UCombatComponent::ReloadShotgunShell()
{
	if (Character && Character->HasAuthority()) UpdateShotgunAmmoValues();
}

// Called from animation blueprint when reload animation completes.
void UCombatComponent::FinishReloading()
{
	if (!Character) return;

	if (Character->HasAuthority())
	{
		UpdateAmmoValues();
		CombatState = ECombatState::ECS_Unoccupied;
	}

	// Attempt to fire again if button is pressed when reload ends.
	if (bFireButtonPressed) FireWeapon();
}

void UCombatComponent::EndShotgunReload()
{
	if (!Character || !Character->GetMesh() || !Character->GetReloadMontage()) return;

	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (!Character || !EquippedWeapon) return;

	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller) Controller->SetHUDCarriedAmmo(CarriedAmmo);

	EquippedWeapon->AddAmmo(-ReloadAmount);
}

// Shotguns reload differently, adding one shell at a time through Animation notifies.
void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (!Character || !EquippedWeapon) return;

	// Update carried ammo.
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller) Controller->SetHUDCarriedAmmo(CarriedAmmo);

	// Update weapon ammo.
	EquippedWeapon->AddAmmo(-1);
	bCanFire = true;

	// Short-circuit reload animation and reload timer.
	if (EquippedWeapon->IsFull() || CarriedAmmo <= 0)
	{
		EndShotgunReload();
		FinishReloading();
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed) FireWeapon();
		break;
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled())
		{
			ShowAttachedGrenade(true);
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
		}
		break;
	default:
		break;
	}
}

//
// Protected Methods
//

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!Character) return;
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

	if (Character->HasAuthority()) InitializeCarriedAmmo();

	if (!Character->GetFollowCamera()) return;
	DefaultFOV = Character->GetFollowCamera()->FieldOfView;
	CurrentFOV = DefaultFOV;
}

void UCombatComponent::SetAiming(bool bAiming)
{
	if (!Character || !EquippedWeapon) return;

	// Initiate aiming on the client (before waiting for replication).
	bIsAiming = bAiming;

	// Call RPC to replicate this action over the network.
	ServerSetAiming(bAiming);

	// Reduce walk speed when aiming.
	Character->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkSpeed;

	if (Character->IsLocallyControlled())
	{
		// Show scope on sniper rifles.
		if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
		{
			Character->ShowSniperScopeWidget(bIsAiming);
		}
	}
}

void UCombatComponent::FireWeapon()
{
	if (!CanFire()) return;

	// Prevent firing until fire timer has elapsed.
	bCanFire = false;

	// Trigger weapon firing on the server authority.
	ServerFire(HitTarget);

	// Save firing state for crosshair spread.
	CrosshairFiringFactor = 0.75f;

	// Enable automatic fire modes by scheduling a callback based on weapon fire rate.
	StartFireTimer();

	// Automatically reload empty weapons 1 second after firing the last round.
	ScheduleAutoReload();
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed) FireWeapon();
}

void UCombatComponent::ThrowGrenade()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;

	if (Character)
	{
		ShowAttachedGrenade(true);
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
	}

	if (!Character->HasAuthority()) ServerThrowGrenade();

	// Use a timer to schedule returning combat state back to Unoccupied.
	StartGrenadeThrowTimer(); // Correct?
}

void UCombatComponent::StartGrenadeThrowTimer()
{
	if (!Character) return;

	Character->GetWorldTimerManager().SetTimer(
		GrenadeThrowTimer,
		this,
		&UCombatComponent::ThrowGrenadeFinished,
		GrenadeThrowDuration
	);
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		ShowAttachedGrenade(true);
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
	}

	// Use a timer to schedule returning combat state back to Unoccupied.
	StartGrenadeThrowTimer(); // Correct?
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	ShowAttachedGrenade(false);
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::ReleaseGrenade()
{
	ShowAttachedGrenade(false);
}

void UCombatComponent::ShowAttachedGrenade(bool bShow)
{
	if (!Character || !Character->GetAttachedGrenade()) return;

	Character->GetAttachedGrenade()->SetVisibility(bShow);
}

void UCombatComponent::ServerSetAiming_Implementation(bool bAiming)
{
	// Initiate aiming on the server authority.
	bIsAiming = bAiming;

	// Reduce walk speed when aiming.
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	// Multicast weapon firing to all clients.
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (!Character || !EquippedWeapon) return;

	if (CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
	else
	{
		if (CombatState == ECombatState::ECS_Reloading &&
			EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
		{
			Character->PlayFireMontage(bIsAiming);
			EquippedWeapon->Fire(TraceHitTarget);
			CombatState = ECombatState::ECS_Unoccupied;
		}
	}
}

void UCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
	if (!GEngine || !GEngine->GameViewport) return;

	// Determine crosshair location.
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	// Translate from world space to local space.
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bWasSuccessful = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);
	if (!bWasSuccessful) return;

	// Run a line trace from the weapon to the target.
	FVector Start = CrosshairWorldPosition;
	if (Character)
	{
		// Start the trace forward to account for the camera boom arm length, and the character mesh.
		float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
		Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
	}
	FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH; // Scale unit vector for long distances.
	GetWorld()->LineTraceSingleByChannel(
		TraceHitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility
	);

	// Debug line traces.
	/*
	UWorld* World = GetWorld();
	if (World)
	{
		// Draw a line trace from the crosshair for debugging.
		DrawDebugLine(World, Start, End, FColor::Red, false, -1.0f, 0, 2.5f);

		// Draw a line trace from the weapon muzzle for debugging.
		if (EquippedWeapon)
		{
			const USkeletalMeshComponent* WeaponMesh = EquippedWeapon->GetWeaponMesh();
			if (WeaponMesh)
			{
				const USkeletalMeshSocket* MuzzleSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
				if (MuzzleSocket)
				{
					FTransform SocketTransform = MuzzleSocket->GetSocketTransform(WeaponMesh);
					FVector MuzzleStart = SocketTransform.GetLocation();
					FVector SocketRotation = SocketTransform.GetRotation().Vector();
					FVector MuzzleEnd = MuzzleStart + SocketRotation * TRACE_LENGTH;
					FVector ToTarget = MuzzleEnd - SocketTransform.GetLocation(); // We use ToTarget.GetRotation() when spawning projectiles.
					DrawDebugLine(World, MuzzleStart, ToTarget, FColor::Magenta, false, -1.0f, 0, 2.5f);
				}
			}
		}
	}
	*/

	// Detect targets under crosshair.
	if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UCrosshairInteractionInterface>())
	{
		HUDPackage.CrosshairColor = FLinearColor::Red;
	}
	else
	{
		HUDPackage.CrosshairColor = FLinearColor::White;
	}

	if (!TraceHitResult.bBlockingHit)
	{
		// Use End position as the result if nothing was hit.
		TraceHitResult.ImpactPoint = End;
	}
}

void UCombatComponent::SetHUDCrosshair(float DeltaTime)
{
	if (!Character || !Character->Controller) return;

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (!Controller) return;

	HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
	if (!HUD) return;

	if (EquippedWeapon)
	{
		// Get crosshair textures from weapon.
		HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCenter;
		HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop;
		HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairBottom;
		HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft;
		HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight;

		// Expand crosshair when moving.
		FVector2D MoveSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
		FVector2D NormalizedRange(0.f, 1.f);
		FVector Velocity = Character->GetVelocity();
		Velocity.Z = 0.f;
		CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(MoveSpeedRange, NormalizedRange, Velocity.Size());

		// Expand crosshair when falling.
		if (Character->GetCharacterMovement()->IsFalling())
		{
			CrosshairFallingFactor = FMath::FInterpTo(CrosshairFallingFactor, 2.f, DeltaTime, 3.f);
		}
		else
		{
			CrosshairFallingFactor = FMath::FInterpTo(CrosshairFallingFactor, 0.f, DeltaTime, 30.f);
		}

		// Shrink crosshair when aiming.
		if (bIsAiming)
		{
			CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, -0.25f, DeltaTime, 30.f);
		}
		else
		{
			CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
		}

		// Shrink crosshair after firing (was expanded in FireButtonPressed()).
		CrosshairFiringFactor = FMath::FInterpTo(CrosshairFiringFactor, 0.f, DeltaTime, 30.f);

		// Set crosshair spread, starting with a base value of 0.25.
		HUDPackage.CrosshairSpread =
			0.25f +
			CrosshairVelocityFactor +
			CrosshairFallingFactor +
			CrosshairAimFactor +
			CrosshairFiringFactor;
	}
	else
	{
		HUDPackage.CrosshairCenter = nullptr;
		HUDPackage.CrosshairTop = nullptr;
		HUDPackage.CrosshairBottom = nullptr;
		HUDPackage.CrosshairLeft = nullptr;
		HUDPackage.CrosshairRight = nullptr;
		HUDPackage.CrosshairSpread = 0.f;
	}

	HUD->SetHUDPackage(HUDPackage);
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (!EquippedWeapon) return;

	if (bIsAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::StartFireTimer()
{
	if (!EquippedWeapon || !Character) return;

	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireInterval
	);
}

void UCombatComponent::FireTimerFinished()
{
	bCanFire = true;

	// Automatically fire again for automatic weapons.
	if (bFireButtonPressed && EquippedWeapon && EquippedWeapon->bAutomaticFireMode)
	{
		FireWeapon();
	}
}

void UCombatComponent::ScheduleAutoReload()
{
	if (!EquippedWeapon || !Character) return;

	Character->GetWorldTimerManager().SetTimer(
		AutoReloadTimer,
		this,
		&UCombatComponent::AutoReloadTimerFinished,
		AutoReloadAfter
	);
}

void UCombatComponent::AutoReloadTimerFinished()
{
	ReloadEmptyWeapon();
}

bool UCombatComponent::CanFire()
{
	if (!EquippedWeapon) {
		// UE_LOG(LogTemp, Warning, TEXT("Cannot fire because no weapon is equipped."));
		return false;
	}
	if (EquippedWeapon->IsEmpty())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Cannot fire because equipped weapon is empty."));
		return false;
	}
	if (!bCanFire)
	{
		// UE_LOG(LogTemp, Warning, TEXT("Cannot fire because FireTimer is incomplete."));
		return false;
	}
	if (CombatState != ECombatState::ECS_Unoccupied)
	{
		// Exception case: Shotguns can fire while reloading.
		if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
			CombatState == ECombatState::ECS_Reloading)
		{
			return true;
		}
		// UE_LOG(LogTemp, Warning, TEXT("Cannot fire because combat state is not UNOCCUPIED."));
		return false;
	}

	return true;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	if (!Character) return;

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller) Controller->SetHUDCarriedAmmo(CarriedAmmo);

	if (CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		CarriedAmmo <= 0)
	{
		EndShotgunReload();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AutomaticRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Submachinegun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_MarksmanRifle, StartingMarksmanAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeAmmo);
}
