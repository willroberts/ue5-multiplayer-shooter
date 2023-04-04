// © 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CrosshairInteractionInterface.generated.h"

UINTERFACE(MinimalAPI)
class UCrosshairInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

// This class will be inherited to implement this interface.
class BLASTERGAME_API ICrosshairInteractionInterface
{
	GENERATED_BODY()
};