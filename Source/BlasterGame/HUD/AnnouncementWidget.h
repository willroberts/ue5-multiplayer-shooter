// (c) 2023 Will Roberts

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "AnnouncementWidget.generated.h"

UCLASS()
class BLASTERGAME_API UAnnouncementWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MatchTimerText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AnnouncementText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* InfoText;
};