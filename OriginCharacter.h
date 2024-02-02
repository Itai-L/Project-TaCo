// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
// #include "Logging/LogMacros.h"
// #include "Logging/Log.h"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OriginCharacter.generated.h"

UCLASS()
class PROJECT_TACO_API AOriginCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AOriginCharacter();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void MoveForward(float Value);
	void RunForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;
	// Declare a variable to track whether the character is running.
	UPROPERTY(VisibleAnywhere)
	bool bDoublePressW;


public:

};
