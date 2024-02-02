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

	bIsInAir = OriginCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = OriginCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
}