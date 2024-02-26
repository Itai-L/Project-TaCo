// Fill out your copyright notice in the Description page of Project Settings.


#include "OriginCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h" // Include this for timers
#include "Components/WidgetComponent.h"



AOriginCharacter::AOriginCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	 CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	 CameraBoom->SetupAttachment(GetMesh());
	 CameraBoom->TargetArmLength = 600.f;
	 CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	//FollowCamera->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "head");
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//UseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidger = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidger->SetupAttachment(RootComponent);


}



// Called when the game starts or when spawned
void AOriginCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void AOriginCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AOriginCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AOriginCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AOriginCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AOriginCharacter::LookUp);

}


void AOriginCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	
	}

}




void AOriginCharacter::RunForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}



void AOriginCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AOriginCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AOriginCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

// Called every frame
void AOriginCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input


void AOriginCharacter::SwitchWeapon(int32 WeaponIndex)
{
	// Check if the weapon index is valid
	if (WeaponIndex >= 0 && WeaponIndex < WeaponClasses.Num())
	{
		// Destroy current weapon
		for (AWeapon* Weapon : Weapons)
		{
			if (Weapon)
			{
				Weapon->Destroy();
			}
		}
		Weapons.Empty();

		// Spawn and attach new weapon
		AWeapon* NewWeapon = GetWorld()->SpawnActor<AWeapon>(WeaponClasses[WeaponIndex], FVector::ZeroVector, FRotator::ZeroRotator);
		if (NewWeapon)
		{
			AttachWeaponToSocket(NewWeapon);
			Weapons.Add(NewWeapon);
		}
	}
}

void AOriginCharacter::AttachWeaponToSocket(AWeapon* Weapon)
{
	if (Weapon && GetMesh())
	{
		USkeletalMeshComponent* MeshComp = GetMesh();
		Weapon->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponSocketName);
	}
}
