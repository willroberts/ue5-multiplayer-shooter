// © 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

UCLASS()
class BLASTERGAME_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* InPawn) override;
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void ShowEliminationPopup(FString Message);
	void HideEliminationPopup();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	/*
	* Popup notification text for eliminations.
	*/

	FTimerHandle EliminationPopupTimer;
	void StartPopupTimer();
	void PopupTimerFinished();
};