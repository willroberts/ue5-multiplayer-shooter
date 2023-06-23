// (c) 2023 Will Roberts

#include "Weapon.h"

#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"

#include "BlasterGame/Character/BlasterCharacter.h"
#include "BlasterGame/Components/CombatComponent.h"
#include "BlasterGame/PlayerController/BlasterPlayerController.h"
#include "ShellCasing.h"

//
// Public Methods
//

AWeapon::AWeapon()
{
	// Configure ticking and replication.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; // Enable network replication.
	SetReplicateMovement(true); // Synchronize actor movement and location.

	// Create WeaponMesh component.
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(WeaponMesh);

	// Set outline highlight color.
	ShowOutlineHighlight(true);

	// Create AreaSphere component (pickup radius).
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); // Will be handled on server only.
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create PickupWidget component.
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget) PickupWidget->SetVisibility(bShowWidget);
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;

	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		ShowOutlineHighlight(false);
		if (WeaponType == EWeaponType::EWT_Submachinegun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		else
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			WeaponMesh->SetEnableGravity(false);
		}
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		ShowOutlineHighlight(true);
		break;
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		WeaponMesh->SetSimulatePhysics(false);
		ShowOutlineHighlight(false);
		if (WeaponType == EWeaponType::EWT_Submachinegun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		else
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			WeaponMesh->SetEnableGravity(false);
		}
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		ShowOutlineHighlight(true);
		break;
	}
}


FVector AWeapon::TraceWithSpread(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (!MuzzleSocket) return FVector();

	// Determine start and end vectors.
	const FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	// Apply spread.
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * SpreadTraceDistance;
	const FVector RandomVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SpreadRadius);
	const FVector EndLocation = SphereCenter + RandomVec;
	const FVector ToEndLocation = EndLocation - TraceStart;
	const FVector WithSpread = FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size());

	/*
	UWorld* World = GetWorld();
	if (World)
	{
		DrawDebugSphere(GetWorld(), SphereCenter, SpreadRadius, 12, FColor::Red, true);
		DrawDebugSphere(GetWorld(), EndLocation, 4.f, 12, FColor::Orange, true);
		DrawDebugLine(GetWorld(), TraceStart, WithSpread, FColor::Cyan, true);
	}
	*/

	return WithSpread;
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation) WeaponMesh->PlayAnimation(FireAnimation, false);

	// Handle case where weapon is missing fire animation (e.g. SMG).
	UWorld* World = GetWorld();
	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashParticles && MuzzleSocket && World)
	{
		FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
		UGameplayStatics::SpawnEmitterAtLocation(World, MuzzleFlashParticles, SocketTransform);
	}
	if (FireSound) UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());

	// Spawn shell casings.
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (World && AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			World->SpawnActor<AShellCasing>(
				CasingClass,
				SocketTransform.GetLocation(),
				SocketTransform.GetRotation().Rotator()
			);
		}
	}

	if (HasAuthority()) ConsumeAmmo();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);

	SetOwner(nullptr);
	OwnerCharacter = nullptr;
	OwnerController = nullptr;
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

//
// Protected Methods
//

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	// Hide pickup widgets initially.
	if (PickupWidget) PickupWidget->SetVisibility(false);
}

void AWeapon::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
) {
	ABlasterCharacter* Character = Cast<ABlasterCharacter>(OtherActor);
	if (Character) Character->SetOverlappingWeapon(this);
}

void AWeapon::OnSphereEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
) {
	ABlasterCharacter* Character = Cast<ABlasterCharacter>(OtherActor);
	if (Character) Character->SetOverlappingWeapon(nullptr);
}

void AWeapon::ConsumeAmmo()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity); // Triggers replication.
	SetHUDAmmo();
}

void AWeapon::OnRep_Ammo()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter && OwnerCharacter->GetCombatComponent() && IsFull())
	{
		OwnerCharacter->GetCombatComponent()->EndShotgunReload();
	}

	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (!Owner)
	{
		OwnerCharacter = nullptr;
		OwnerController = nullptr;
		return;
	}
}

void AWeapon::SetHUDAmmo()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter) return;

	OwnerController = OwnerController == nullptr ? Cast<ABlasterPlayerController>(OwnerCharacter->Controller) : OwnerController;
	if (!OwnerController) return;

	OwnerController->SetHUDWeaponAmmo(Ammo);
}

void AWeapon::SetHUDWeaponType()
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
	if (!OwnerCharacter) return;

	OwnerController = OwnerController == nullptr ? Cast<ABlasterPlayerController>(OwnerCharacter->Controller) : OwnerController;
	if (!OwnerController) return;

	OwnerController->SetHUDWeaponType(GetWeaponType());
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

bool AWeapon::IsFull()
{
	return Ammo >= MagCapacity;
}

void AWeapon::ShowOutlineHighlight(bool bShow)
{
	if (!WeaponMesh) return;

	if (bShow)
	{
		int32 Color;
		switch (WeaponRarity)
		{
		case EWeaponRarity::EWR_Legendary:
			Color = OUTLINE_HIGHLIGHT_PURPLE;
			break;
		case EWeaponRarity::EWR_Rare:
			Color = OUTLINE_HIGHLIGHT_BLUE;
			break;
		default:
			Color = OUTLINE_HIGHLIGHT_TAN;
			break;
		}
		WeaponMesh->SetCustomDepthStencilValue(Color);
		WeaponMesh->MarkRenderStateDirty();
	}

	WeaponMesh->SetRenderCustomDepth(bShow);
}