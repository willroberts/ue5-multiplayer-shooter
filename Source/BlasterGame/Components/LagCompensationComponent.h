// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
    GENERATED_BODY()

    UPROPERTY()
    FVector Location;

    UPROPERTY()
    FRotator Rotation;

    UPROPERTY()
    FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
    GENERATED_BODY()

    UPROPERTY()
    float Time;

    UPROPERTY()
    TMap<FName, FBoxInformation> HitboxInfo;
};

UCLASS()
class BLASTERGAME_API ULagCompensationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULagCompensationComponent();
    friend class ABlasterCharacter;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    ABlasterCharacter* Character;

    UPROPERTY()
    class ABlasterPlayerController* Controller;
};