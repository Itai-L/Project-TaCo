// Fill out your copyright notice in the Description page of Project Settings.

#include "OriginAnimInstance.h"
#include "OriginCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UOriginAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OriginCharacter = Cast<AOriginCharacter>(TryGetPawnOwner());
}

void UOriginAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (OriginCharacter == nullptr)
	{
		OriginCharacter = Cast<AOriginCharacter>(TryGetPawnOwner());
	}
	if (OriginCharacter == nullptr) return;

	FVector Velocity = OriginCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	if (Speed < 0.0f)
	{
		// Speed is negative, meaning the character is moving backwards
		bIsMovingBackwards = true;
	}
	else
	{
		// Speed is non-negative, meaning the character is not moving backwards
		bIsMovingBackwards = false;
	}

	bIsInAir = OriginCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = OriginCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bAiming= OriginCharacter->isAiming();
	bScanStance = OriginCharacter->isScanStance();
	bGeneralStance = OriginCharacter->isGeneralStance();
	bCloseQuarters = OriginCharacter->isCloseQuarters();
	bIsCrouched = OriginCharacter->bIsCrouched;
	bInProne = OriginCharacter->bInProne;
	AO_Yaw = OriginCharacter->GetAO_Yaw();
	AO_Pitch = OriginCharacter->GetAO_Pitch();
	LeftHandTransform = OriginCharacter->getLeftHandTransform();
	bTurnRight = OriginCharacter->getTurnRight();
	bTurnLeft = OriginCharacter->getTurnLeft();
	TurningInPlace = OriginCharacter->GetTurningInPlace();


}