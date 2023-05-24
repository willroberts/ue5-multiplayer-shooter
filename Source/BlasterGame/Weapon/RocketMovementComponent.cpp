// (c) 2023 Will Roberts

#include "RocketMovementComponent.h"

URocketMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(
	const FHitResult& Hit,
	float TimeTick,
	const FVector& MoveDelta,
	float& SubTickTimeRemaining
) {
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);

	// Allow rockets to continue on blocking hit.
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void URocketMovementComponent::HandleImpact(
	const FHitResult& Hit,
	float TimeSlice,
	const FVector& MoveDelta
) {
	// Rockets should not stop; only explode when CollisionBox detects a hit.
	return;

	// Super::HandleImpact(Hit, TimeSlice, MoveDelta);
}