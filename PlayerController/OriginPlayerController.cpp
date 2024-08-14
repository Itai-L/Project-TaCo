// Fill out your copyright notice in the Description page of Project Settings.


#include "OriginPlayerController.h"

#include "Project_Taco/HUD/OriginHUD.h"
#include "Project_Taco/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void AOriginPlayerController::BeginPlay()
{
	Super::BeginPlay();

	OriginHUD = Cast<AOriginHUD>(GetHUD());
}

void AOriginPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	OriginHUD = OriginHUD == nullptr ? Cast<AOriginHUD>(GetHUD()) : OriginHUD;

	bool bHUDValid = OriginHUD && OriginHUD->CharacterOverlay && OriginHUD->CharacterOverlay->HealthBar && OriginHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		OriginHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		OriginHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void AOriginPlayerController::SetHUDFatigue(float Fatigue)
{
	OriginHUD = OriginHUD == nullptr ? Cast<AOriginHUD>(GetHUD()) : OriginHUD;

	bool bHUDValid = OriginHUD && OriginHUD->CharacterOverlay && OriginHUD->CharacterOverlay->FatigueBar && OriginHUD->CharacterOverlay->FatigueText;
	if (bHUDValid)
	{
		// Update the fatigue text
		FString FatigueText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Fatigue));
		OriginHUD->CharacterOverlay->FatigueText->SetText(FText::FromString(FatigueText));

		// Update the fatigue bar (assuming it's a progress bar or slider)
		float FatiguePercentage = FMath::Clamp(Fatigue / 100.0f, 0.0f, 1.0f);
		OriginHUD->CharacterOverlay->FatigueBar->SetPercent(FatiguePercentage);
	}
}