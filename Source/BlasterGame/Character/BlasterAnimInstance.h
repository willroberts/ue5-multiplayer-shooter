// © 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BlasterAnimInstance.generated.h"

UCLASS()
class BLASTERGAME_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = true))
	class ABlasterCharacter* BlasterCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = true))
	bool bWeaponEquipped;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = true))
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float YawOffset;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float Lean;

	FRotator CharacterRotation;
	FRotator CharacterRotationLastFrame;
	FRotator DeltaRotation;
};