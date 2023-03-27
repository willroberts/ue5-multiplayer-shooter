// © 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShellCasing.generated.h"

UCLASS()
class BLASTERGAME_API AShellCasing : public AActor
{
	GENERATED_BODY()
	
public:	
	AShellCasing();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;
};