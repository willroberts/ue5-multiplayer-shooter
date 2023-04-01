// © 2023 Will Roberts

#include "CombatComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "BlasterGame/Weapon/Weapon.h"
#include "BlasterGame/Character/BlasterCharacter.h"
#include "BlasterGame/PlayerController/BlasterPlayerController.h"
#include "BlasterGame/HUD/BlasterHUD.h"

#define TRACE_LENGTH 80000

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
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!Character || !WeaponToEquip)
	{
		return;
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

//
// Protected Methods
//

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!Character)
	{
		return;
	}
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

	if (!Character->GetFollowCamera())
	{
		return;
	}
	DefaultFOV = Character->GetFollowCamera()->FieldOfView;
	CurrentFOV = DefaultFOV;
}

void UCombatComponent::SetAiming(bool bAiming)
{
	// Initiate aiming on the client (before waiting for replication).
	bIsAiming = bAiming;

	// Call RPC to replicate this action over the network.
	ServerSetAiming(bAiming);

	// Reduce walk speed when aiming.
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		// Trigger weapon firing on the server authority.
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult);
		ServerFire(HitResult.ImpactPoint);
	}
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
	if (Character && EquippedWeapon)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
	if (!GEngine || !GEngine->GameViewport)
	{
		// Failed to get viewport!
		return;
	}

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
	if (!bWasSuccessful)
	{
		// Failed to translate world space!
		return;
	}

	// Run a line trace from the weapon to the target.
	FVector Start = CrosshairWorldPosition;
	FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH; // Scale unit vector for long distances.
	GetWorld()->LineTraceSingleByChannel(
		TraceHitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility
	);

	if (!TraceHitResult.bBlockingHit)
	{
		// Use End position as the result if nothing was hit.
		TraceHitResult.ImpactPoint = End;
	}
}

void UCombatComponent::SetHUDCrosshair(float DeltaTime)
{
	if (!Character || !Character->Controller)
	{
		return;
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (!Controller)
	{
		return;
	}

	HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
	if (!HUD)
	{
		return;
	}

	FHUDPackage HUDPackage;
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

		// Set crosshair spread.
		HUDPackage.CrosshairSpread = CrosshairVelocityFactor + CrosshairFallingFactor;
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
	if (!EquippedWeapon)
	{
		return;
	}

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