// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverHeadUserWidger.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_TACO_API UOverHeadUserWidger : public UUserWidget
{
	GENERATED_BODY()
	public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	void SetDisplayText(FString TextToDisplay);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);

protected:
	//virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;
	
};
