// (c) 2023 Will Roberts

#include "BuffComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BlasterGame/Character/BlasterCharacter.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RestoreHealthOverTime(DeltaTime);
	RestoreShieldOverTime(DeltaTime);
}

void UBuffComponent::RestoreHealth(float Amount, float Duration)
{
	bHealing = true;
	HealRate = Amount / Duration;
	HealAmount += Amount;
}

void UBuffComponent::RestoreHealthOverTime(float DeltaTime)
{
	if (!bHealing) return;
	if (!Character) return;
	if (Character->IsEliminated()) return;

	// Restore health this frame.
	const float HealAmountThisFrame = HealRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(
		Character->GetHealth() + HealAmountThisFrame,
		0.f,
		Character->GetMaxHealth()
	));
	Character->UpdateHUDHealth();
	HealAmount -= HealAmountThisFrame;

	// Stop healing at max health.
	if (HealAmount <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		HealAmount = 0.f;
	}
}

void UBuffComponent::RestoreShield(float Amount, float Duration)
{
	bShieldRecharging = true;
	ShieldRechargeRate = Amount / Duration;
	ShieldRechargeAmount += Amount;
}

void UBuffComponent::RestoreShieldOverTime(float DeltaTime)
{
	if (!bShieldRecharging) return;
	if (!Character) return;
	if (Character->IsEliminated()) return;

	// Restore shield this frame.
	const float ShieldRechargeAmountThisFrame = ShieldRechargeRate * DeltaTime;
	Character->SetShield(FMath::Clamp(
		Character->GetShield() + ShieldRechargeAmountThisFrame,
		0.f,
		Character->GetMaxShield()
	));
	Character->UpdateHUDShield();
	ShieldRechargeAmount -= ShieldRechargeAmountThisFrame;

	// Stop restoring shield at max shield.
	if (ShieldRechargeAmount <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bShieldRecharging = false;
		ShieldRechargeAmount = 0.f;
	}
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float Magnitude)
{
	if (!Character) return;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = Character->GetStandingMoveSpeed() * Magnitude;
		MoveComp->MaxWalkSpeedCrouched = Character->GetCrouchedMoveSpeed() * Magnitude;
	}
}

void UBuffComponent::ApplySpeedBuff(float Magnitude, float Duration)
{
	if (!Character) return;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = Character->GetStandingMoveSpeed() * Magnitude;
		MoveComp->MaxWalkSpeedCrouched = Character->GetCrouchedMoveSpeed() * Magnitude;
	}
	MulticastSpeedBuff(Magnitude);

	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::SpeedBuffTimerFinished, Duration);
}

void UBuffComponent::SpeedBuffTimerFinished()
{
	if (!Character) return;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = Character->GetStandingMoveSpeed();
		MoveComp->MaxWalkSpeedCrouched = Character->GetCrouchedMoveSpeed();
	}
	MulticastSpeedBuff(1.0f);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float Magnitude)
{
	if (!Character) return;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->JumpZVelocity = Character->GetJumpSpeed() * Magnitude;
	}
}

void UBuffComponent::ApplyJumpBuff(float Magnitude, float Duration)
{
	if (!Character) return;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->JumpZVelocity = Character->GetJumpSpeed() * Magnitude;
	}
	MulticastJumpBuff(Magnitude);

	Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &UBuffComponent::JumpBuffTimerFinished, Duration);
}

void UBuffComponent::JumpBuffTimerFinished()
{
	if (!Character) return;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->JumpZVelocity = Character->GetJumpSpeed();
	}
	MulticastJumpBuff(1.0f);
}