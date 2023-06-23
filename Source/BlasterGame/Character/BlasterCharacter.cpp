// (c) 2023 Will Roberts

#include "BlasterCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

#include "BlasterAnimInstance.h"
#include "BlasterGame/BlasterGame.h"
#include "BlasterGame/Components/BuffComponent.h"
#include "BlasterGame/Components/CombatComponent.h"
#include "BlasterGame/GameModes/BlasterGameMode.h"
#include "BlasterGame/PlayerController/BlasterPlayerController.h"
#include "BlasterGame/PlayerState/BlasterPlayerState.h"
#include "BlasterGame/Weapon/Weapon.h"
#include "BlasterGame/Weapon/WeaponTypes.h"

//
// Public Methods
//

ABlasterCharacter::ABlasterCharacter()
{
	// Configure ticking and replication.
	PrimaryActorTick.bCanEverTick = true;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	// Configure movement.
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 720.f);
	GetCharacterMovement()->MaxWalkSpeed = StandingMoveSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchedMoveSpeed;
	TurningInPlace = ETurningInPlace::ETIP_NotTurning; // Initialize turning state.

	// Configure collision.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Create Camera components.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Create OverheadWidget component.
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	// Create components.
	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	// Configure dissolve VFX.
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	// Create grenade mesh.
	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Construct hitboxes.
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	head->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	pelvis->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	spine_02->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	spine_03->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	upperarm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	upperarm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	lowerarm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	lowerarm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	hand_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	hand_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	backpack->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	blanket->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	thigh_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	thigh_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	calf_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	calf_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	foot_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	foot_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Replicates character rotation and aim offsets.
	RotateInPlace(DeltaTime);

	// Make the Character invisible when the Camera is too close, obscuring vision.
	CameraHideMesh();

	// Poll player state each frame until it becomes valid, then skip further processing.
	PollPlayerState();
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);
	DOREPLIFETIME(ABlasterCharacter, bDisableCombatActions);
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &ABlasterCharacter::DropButtonPressed);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ABlasterCharacter::GrenadeButtonPressed);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (BuffComponent) BuffComponent->Character = this;
	if (Combat) Combat->Character = this;
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	// Handle the case where a player is hosting a listen server, and does not receive a RepNotify.
	// Hide widget if overlap has ended.
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon) OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon; // Writing to this attribute triggers replication.

	// Handle the case where a player is hosting a listen server, and does not receive a RepNotify.
	// Show widget if overlap has begun.
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon) OverlappingWeapon->ShowPickupWidget(true);
	}
}

// When overlap begins, OverlappingWeapon != nullptr and LastWeapon == nullptr.
// When overlap ends, OverlappingWeapon == nullptr and LastWeapon != nullptr.
void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	// Overlap began.
	if (OverlappingWeapon) OverlappingWeapon->ShowPickupWidget(true);

	// Overlap ended.
	if (LastWeapon)	LastWeapon->ShowPickupWidget(false);
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bIsAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (Combat && Combat->EquippedWeapon) return Combat->EquippedWeapon;

	return nullptr;
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (!Combat || !Combat->EquippedWeapon)	return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (!Combat || !Combat->EquippedWeapon)	return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AutomaticRifle:
			SectionName = FName("AutomaticRifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Submachinegun:
			SectionName = FName("SubmachineGun");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_MarksmanRifle:
			SectionName = FName("MarksmanRifle");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		default:
			// Use the default rifle reload animation as a fallback.
			SectionName = FName("AutomaticRifle");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayEliminatedMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EliminatedMontage)
	{
		AnimInstance->Montage_Play(EliminatedMontage);
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	// Playing this animation requires an equipped weapon.
	if (!Combat) return;
	if (!Combat->EquippedWeapon) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (!Combat) return FVector(); // unsafe???

	return Combat->HitTarget;
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	TimeSinceLastMovementRep = 0.f;
}

// Eliminated runs on the server authority to handle player eliminations and respawning.
void ABlasterCharacter::Eliminated()
{
	// Hide the scope if in use.
	if (IsLocallyControlled()
		&& Combat
		&& Combat->bIsAiming
		&& Combat->EquippedWeapon
		&& Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		ShowSniperScopeWidget(false);
	}

	// Drop any equipped weapon, except for the starter pistol.
	if (Combat && Combat->EquippedWeapon)
	{
		if (Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol)
		{
			Combat->EquippedWeapon->Destroy();
		}
		else
		{
			Combat->UnequipWeapon();
		}
	}

	MulticastEliminated();
	GetWorldTimerManager().SetTimer(RespawnTimer, this, &ABlasterCharacter::RespawnTimerFinished, RespawnDelay);
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (RespawnBotComponent) RespawnBotComponent->DestroyComponent();

	// Handle the case where a player is destroyed by disconnecting rather than by elimination.
	if (Combat && Combat->EquippedWeapon) Combat->UnequipWeapon();
}

// Server+Client RPC.
void ABlasterCharacter::MulticastEliminated_Implementation()
{
	bEliminated = true;

	PlayEliminatedMontage();

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponType(EWeaponType::EWT_MAX);
		BlasterPlayerController->SetHUDWeaponAmmo(0);
		BlasterPlayerController->SetHUDCarriedAmmo(0);
	}

	// Apply dissolve VFX to the eliminated character.
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);

		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("DissolveMagnitude"), 0.55f); // Undissolved.
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("GlowIntensity"), 200.f);
	}
	StartDissolve();

	// Disable movement and collision.
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (BlasterPlayerController) DisableInput(BlasterPlayerController);
	if (Combat) Combat->FireButtonPressed(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Spawn the respawn bot.
	if (RespawnBotEffect)
	{
		RespawnBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			RespawnBotEffect,
			FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f),
			GetActorRotation()
		);
	}
	if (RespawnBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, RespawnBotSound, GetActorLocation());
	}

	// Hide the scope if in use.
	if (IsLocallyControlled()
		&& Combat
		&& Combat->bIsAiming
		&& Combat->EquippedWeapon
		&& Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		ShowSniperScopeWidget(false);
	}
}

//
// Protected Methods
//

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority()) OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);

	SpawnDefaultWeapon();
	if (AttachedGrenade) AttachedGrenade->SetVisibility(false);

	UpdateHUDHealth();
	UpdateHUDShield();
	UpdateHUDAmmo();
}

void ABlasterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
		return;
	}
	Super::Jump();
}

void ABlasterCharacter::MoveForward(float Value)
{
	if (Value == 0.f) return;

	if (Controller)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value); // NOTE: Set speed/acceleration in MovementComponent.
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (Value == 0.f) return;

	if (Controller)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	// Applies to ROLE_AutonomousProxy and ROLE_Authority.
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		// Store look orientation for aim offset tracking.
		AimOffset(DeltaTime);
	}
	else
	{
		// Keep track of movement replication to ensure regular updates.
		TimeSinceLastMovementRep += DeltaTime;
		if (TimeSinceLastMovementRep > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void ABlasterCharacter::Turn(float Value)
{
	if (Value == 0.f) return;

	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	if (Value == 0.f) return;

	AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()
{
	if (bDisableCombatActions) return;

	if (!HasAuthority())
	{
		ServerEquipButtonPressed();
		return;
	}

	if (Combat) Combat->EquipWeapon(OverlappingWeapon);
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat) Combat->EquipWeapon(OverlappingWeapon);
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bIsCrouched) UnCrouch();
	else {
		// Prevent crouching mid-jump.
		if (!GetCharacterMovement()->IsFalling()) Crouch();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if (Combat)	Combat->SetAiming(true);
}

void ABlasterCharacter::AimButtonReleased()
{
	if (Combat)	Combat->SetAiming(false);
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (bDisableCombatActions) return;

	if (Combat) Combat->Reload();
}

void ABlasterCharacter::GrenadeButtonPressed()
{
	if (bDisableCombatActions) return;

	if (Combat) Combat->ThrowGrenade();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	// Compute aim pitch offset.
	AO_Pitch = GetBaseAimRotation().Pitch;

	// Pitch correction for replicated characters.
	// Pitch is compressed before replication, mapping -90 to 270, for example.
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	// Nothing to calculate when no weapon is equipped.
	if (Combat && !Combat->EquippedWeapon) return;

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	// If not moving or jumping, compute aim yaw offset.
	if (Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime); // Rotate the character if turning without moving.
	}

	// If moving or jumping, save last aim rotation.
	if (Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

// Interpolate turning-in-place on simulated proxies.
void ABlasterCharacter::SimProxiesTurn()
{
	if (!Combat || !Combat->EquippedWeapon)	return;

	bRotateRootBone = false; // Is 'true' on locally controlled pawns.
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnInPlaceThreshold)
	{
		if (ProxyYaw > TurnInPlaceThreshold) TurningInPlace = ETurningInPlace::ETIP_Right;
		else if (ProxyYaw < -TurnInPlaceThreshold) TurningInPlace = ETurningInPlace::ETIP_Left;
		else TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::FireButtonPressed()
{
	if (bDisableCombatActions) return;

	if (Combat)	Combat->FireButtonPressed(true);
}

void ABlasterCharacter::FireButtonReleased()
{
	if (Combat)	Combat->FireButtonPressed(false);
}

void ABlasterCharacter::DropButtonPressed()
{
	if (bDisableCombatActions) return;

	if (!HasAuthority())
	{
		ServerDropButtonPressed();
		return;
	}

	if (Combat && Combat->EquippedWeapon) Combat->UnequipWeapon();
}

void ABlasterCharacter::ServerDropButtonPressed_Implementation()
{
	if (Combat && Combat->EquippedWeapon) Combat->UnequipWeapon();
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (!Combat || !Combat->EquippedWeapon) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName = FName("FromFront"); // FIXME: 4 directions.
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController) BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
}

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController) BlasterPlayerController->SetHUDShield(Shield, MaxShield);
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	if (!Combat || !Combat->EquippedWeapon) return;

	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (!BlasterPlayerController) return;
	
	BlasterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	BlasterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
}

void ABlasterCharacter::SpawnDefaultWeapon()
{
	if (bEliminated) return;
	if (!DefaultWeaponClass) return;

	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GameMode) return;

	UWorld* World = GetWorld();
	if (!World) return;
	
	AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
	if (Combat) Combat->EquipWeapon(StartingWeapon);

	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (!BlasterPlayerController) return;

	BlasterPlayerController->SetHUDWeaponType(StartingWeapon->GetWeaponType());
}

bool ABlasterCharacter::IsLocallyReloading()
{
	if (!Combat) return false;

	return Combat->bIsLocallyReloading;
}

void ABlasterCharacter::PollPlayerState()
{
	if (BlasterPlayerState) return;

	BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
	if (BlasterPlayerState)
	{
		// Player state is now valid; update HUD.
		BlasterPlayerState->AddToScore(0.f);
		BlasterPlayerState->AddToDefeats(0);
	}
}

// Called on the server when a character takes damage.
void ABlasterCharacter::ReceiveDamage(
	AActor* DamagedActor,
	float Damage,
	const UDamageType* DamageType,
	class AController* InstigatorController,
	AActor* DamageCauser
)
{
	if (bEliminated) return;

	// Remove damage from shield first.
	float RemainingDamage = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			RemainingDamage = 0.f;
		}
		else
		{
			RemainingDamage = FMath::Clamp(Damage - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}

	// Remove damage from health second.
	Health = FMath::Clamp(Health - RemainingDamage, 0.f, MaxHealth);

	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackingController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackingController);
		}
	}
}

//
// Private Methods
//

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	// When orientation is above 90 degrees to the left or right, set TurningInPlace to that direction.
	if (AO_Yaw > 90.f) TurningInPlace = ETurningInPlace::ETIP_Right;
	else if (AO_Yaw < -90.f) TurningInPlace = ETurningInPlace::ETIP_Left;

	// When turning, interpolate yaw.
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;

		// Stop turning when angle is less than 15 degrees from forward.
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

// Hide the Character Mesh when the Camera boom arm is too short, preventing clipping.
void ABlasterCharacter::CameraHideMesh()
{
	if (!IsLocallyControlled())	return;

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraHideMeshThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

// Runs on clients when damage is taken.
void ABlasterCharacter::OnRep_Health(float PreviousValue)
{
	UpdateHUDHealth();

	if (Health < PreviousValue) PlayHitReactMontage();
}

// Runs on clients when damage is taken.
void ABlasterCharacter::OnRep_Shield(float PreviousValue)
{
	UpdateHUDShield();

	if (Shield < PreviousValue) PlayHitReactMontage();
}

void ABlasterCharacter::RespawnTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveMagnitude)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("DissolveMagnitude"), DissolveMagnitude);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveTimeline && DissolveCurve)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (!Combat) return ECombatState::ECS_MAX; // Something went wrong!
	
	return Combat->CombatState;
}