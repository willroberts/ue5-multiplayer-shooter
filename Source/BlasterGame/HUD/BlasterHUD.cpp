// © 2023 Will Roberts

#include "BlasterHUD.h"

#include "GameFramework/PlayerController.h"

#include "CharacterOverlay.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();

	AddCharacterOverlay();
}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		if (!HUDPackage.CrosshairCenter ||
			!HUDPackage.CrosshairTop ||
			!HUDPackage.CrosshairBottom ||
			!HUDPackage.CrosshairLeft ||
			!HUDPackage.CrosshairRight)
		{
			// Missing textures!
			return;
		}

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		DrawCrosshair(HUDPackage.CrosshairCenter, ViewportCenter, FVector2D(0.f, 0.f), HUDPackage.CrosshairColor);
		DrawCrosshair(HUDPackage.CrosshairTop, ViewportCenter, FVector2D(0.f, -SpreadScaled), HUDPackage.CrosshairColor);
		DrawCrosshair(HUDPackage.CrosshairBottom, ViewportCenter, FVector2D(0.f, SpreadScaled), HUDPackage.CrosshairColor);
		DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCenter, FVector2D(-SpreadScaled, 0.f), HUDPackage.CrosshairColor);
		DrawCrosshair(HUDPackage.CrosshairRight, ViewportCenter, FVector2D(SpreadScaled, 0.f), HUDPackage.CrosshairColor);
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor Color)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X, TextureDrawPoint.Y,
		TextureWidth, TextureHeight,
		0.f, 0.f,
		1.f, 1.f,
		Color
	);
}