// © 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

UCLASS()
class BLASTERGAME_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void AddToScore(float Value);
	virtual void OnRep_Score() override;
	void AddToDefeats(int32 Value);

	UFUNCTION()
	virtual void OnRep_Defeats();

	void SetEliminationPopup(FString Message, FLinearColor Color);

	UFUNCTION()
	virtual void OnRep_EliminationPopup();

private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	UPROPERTY(ReplicatedUsing = OnRep_EliminationPopup)
	FString EliminationPopupText = "";
	FLinearColor EliminationPopupColor = FLinearColor::White;
};