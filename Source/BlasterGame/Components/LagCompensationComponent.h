// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LagCompensationComponent.generated.h"

UCLASS()
class BLASTERGAME_API ALagCompensationComponent : public AActor
{
	GENERATED_BODY()
	
public:	
	ALagCompensationComponent();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

private:	
};