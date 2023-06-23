// (c) 2023 Will Roberts

#include "LagCompensationComponent.h"

ALagCompensationComponent::ALagCompensationComponent()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ALagCompensationComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}