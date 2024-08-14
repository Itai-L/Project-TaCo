// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "OriginHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:

};

/**
 *
 */
UCLASS()
class PROJECT_TACO_API AOriginHUD : public AHUD
{
	GENERATED_BODY()
public:

	virtual void DrawHUD() override;
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	class UCharacterOverlay* CharacterOverlay;

protected:

	virtual void BeginPlay() override;
	void AddCharacterOverlay();

private:
	FHUDPackage HUDPackage;


public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};