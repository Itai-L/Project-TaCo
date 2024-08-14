// Fill out your copyright notice in the Description page of Project Settings.


#include "OriginHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"

void AOriginHUD::BeginPlay()
{
	Super::BeginPlay();

	AddCharacterOverlay();
}

void AOriginHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void AOriginHUD::DrawHUD()
{
	Super::DrawHUD();

}

