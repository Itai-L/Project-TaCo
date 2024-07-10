// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "../Project_Taco/TurningInPlace.h"
#include "OriginAnimInstance.generated.h"


/**
 * 
 */
UCLASS()
class PROJECT_TACO_API UOriginAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class AOriginCharacter* OriginCharacter;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsMovingBackwards;
	UPROPERTY(BlueprintReadOnly, Category = Shooting, meta = (AllowPrivateAccess = "true"))
	bool bAiming;
	UPROPERTY(BlueprintReadOnly, Category = Stance, meta = (AllowPrivateAccess = "true"))
	bool bGeneralStance;
	UPROPERTY(BlueprintReadOnly, Category = Stance, meta = (AllowPrivateAccess = "true"))
	bool bScanStance;
	UPROPERTY(BlueprintReadOnly, Category = Stance, meta = (AllowPrivateAccess = "true"))
	bool bCloseQuarters;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bInProne;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bTurnRight;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bTurnLeft;

	UPROPERTY(BlueprintReadOnly, Category = Stance, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	ETurningInPlace TurningInPlace;

	
};

