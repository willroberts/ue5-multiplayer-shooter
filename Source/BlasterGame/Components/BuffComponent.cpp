// (c) 2023 Will Roberts

#include "BuffComponent.h"
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
}

void UBuffComponent::RestoreHealth(float Amount, float Duration)
{
	bHealing = true;
	HealRate = Amount / Duration;
	HealAmount += Amount;
}

void UBuffComponent::RestoreHealthOverTime(float DeltaTime)
{
	if (!Character) return;
	if (Character->IsEliminated()) return;
	if (!bHealing) return;

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