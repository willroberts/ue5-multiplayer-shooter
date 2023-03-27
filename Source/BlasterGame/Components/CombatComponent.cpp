// © 2023 Will Roberts

#include "CombatComponent.h"

#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "BlasterGame/Weapon/Weapon.h"
#include "BlasterGame/Character/BlasterCharacter.h"

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

	FHitResult HitResult;
	TraceUnderCrosshair(HitResult);
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

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
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
		ServerFire();
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

void UCombatComponent::ServerFire_Implementation()
{
	// Multicast weapon firing to all clients.
	MulticastFire();
}

void UCombatComponent::MulticastFire_Implementation()
{
	if (Character && EquippedWeapon)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire();
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
	else
	{
		// Draw a debug sphere at the point of impact.
		DrawDebugSphere(
			GetWorld(),
			TraceHitResult.ImpactPoint,
			12.f,
			12,
			FColor::Red
		);
	}
}