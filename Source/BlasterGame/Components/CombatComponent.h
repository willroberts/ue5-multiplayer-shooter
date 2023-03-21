// © 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTERGAME_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* Weapon);

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming);

private:	
	class ABlasterCharacter* Character;

	UPROPERTY(Replicated)
	class AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bIsAiming;
};