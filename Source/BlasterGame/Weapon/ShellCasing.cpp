// © 2023 Will Roberts

#include "ShellCasing.h"

AShellCasing::AShellCasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
}

void AShellCasing::BeginPlay()
{
	Super::BeginPlay();
}