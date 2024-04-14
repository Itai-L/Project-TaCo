// Fill out your copyright notice in the Description page of Project Settings.


#include "OriginCharacter.h"
#include "Weapon/Weapon.h"
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
	PlayerInputComponent->BindAction("RightMouseButton", IE_Pressed, this, &AOriginCharacter::OnRightMousePressed);
	PlayerInputComponent->BindAction("RightMouseButton", IE_Released, this, &AOriginCharacter::OnRightMouseReleased);
	PlayerInputComponent->BindAction("LeftCtrl", IE_Pressed, this, &AOriginCharacter::LeftCtrlPressed);
	PlayerInputComponent->BindAction("Wkey", IE_Pressed, this, &AOriginCharacter::WisPressed);
	PlayerInputComponent->BindAction("Wkey", IE_Released, this, &AOriginCharacter::WisReleased);
	PlayerInputComponent->BindAction("LeftShift", IE_Pressed, this, &AOriginCharacter::LeftShiftPressed);
	PlayerInputComponent->BindAction("LeftShift", IE_Released, this, &AOriginCharacter::LeftShiftReleased);
	PlayerInputComponent->BindAction("OneKey", IE_Released, this, &AOriginCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("TwoKey", IE_Released, this, &AOriginCharacter::TwoKeyPressed);
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

void AOriginCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOriginCharacter::ResetPressCount()
{
	PressedAmount = 0;
	//GetCharacterMovement()->MaxWalkSpeed = PreviousSpeed; // Restore speed if needed

}

void AOriginCharacter::SwitchWeapon(int32 WeaponIndex)
{
	if (HasAuthority())
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
	else if (GetLocalRole() < ROLE_Authority)
	{
		ServerSwitchWeapon(WeaponIndex);
	}
}

bool AOriginCharacter::isAiming()
{
	return bAiming;
}

bool AOriginCharacter::isGeneralStance()
{
	
	return bGeneralStance;
}

bool AOriginCharacter::isScanStance()
{
	return bScanStance;
}

bool AOriginCharacter::isCloseQuarters()
{
	return bCloseQuarters;
}

void AOriginCharacter::AttachWeaponToSocket(AWeapon* Weapon)
{
	if (Weapon && GetMesh())
	{
		USkeletalMeshComponent* MeshComp = GetMesh();
		Weapon->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponSocketName);
	}
}

void AOriginCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOriginCharacter, bAiming);
	DOREPLIFETIME(AOriginCharacter, bGeneralStance);
	DOREPLIFETIME(AOriginCharacter, bScanStance);
	DOREPLIFETIME(AOriginCharacter, bCloseQuarters);
	DOREPLIFETIME(AOriginCharacter, bWKey);
	DOREPLIFETIME(AOriginCharacter, WalkSpeed);
	DOREPLIFETIME(AOriginCharacter, RunSpeed);
	DOREPLIFETIME(AOriginCharacter, PreviousSpeed);
	DOREPLIFETIME(AOriginCharacter, CurrentWeaponIndex);



}

void AOriginCharacter::OnRightMousePressed()
{
	if (HasAuthority())
	{
		// Set Aim variable to true
		bAiming = true;

	}
	else if (GetLocalRole() < ROLE_Authority)
	{
		// If not authority, send request to server
		ServerOnRightMousePressed();
	}
}

void AOriginCharacter::ServerOnRightMousePressed_Implementation()
{
	// Set Aim variable to true on the server
	bAiming = true;
}

bool AOriginCharacter::ServerOnRightMousePressed_Validate()
{
	return true;
}

void AOriginCharacter::OnRightMouseReleased()
{
	if (HasAuthority())
	{
		// Set Aim variable to false
		bAiming = false;
	}
	else if (GetLocalRole() < ROLE_Authority)
	{
		// If not authority, send request to server
		ServerOnRightMouseReleased();
	}
}

void AOriginCharacter::ServerOnRightMouseReleased_Implementation()
{
	// Set Aim variable to false on the server
	bAiming = false;
}

bool AOriginCharacter::ServerOnRightMouseReleased_Validate()
{
	return true;
}

void AOriginCharacter::LeftCtrlPressed()
{
	if (HasAuthority()) {

		if (bGeneralStance == false && bScanStance == false && bCloseQuarters == false) bGeneralStance = true;

		if (bGeneralStance == true)
		{
			bGeneralStance = false;
			bScanStance = true;
			bCloseQuarters = false;
			WalkSpeed = 200;
			RunSpeed = 400;
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			return;
		}

		if (bScanStance == true)
		{
			bGeneralStance = false;
			bScanStance = false;
			bCloseQuarters = true;
			WalkSpeed = 100;
			RunSpeed = 300;
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			return;

		}

		if (bCloseQuarters == true)
		{
			bGeneralStance = true;
			bScanStance = false;
			bCloseQuarters = false;
			WalkSpeed = 300;
			RunSpeed = 600;
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			return;
		}
	}
		else if (GetLocalRole() < ROLE_Authority)
		{
			// If not authority, send request to server
			ServerLeftCtrlPressed();
		}

}

void AOriginCharacter::WisPressed()
{
	if (HasAuthority())
	{
		bWKey = true;
		PressedAmount++;
		if (PressedAmount == 1)
		{
			PreviousSpeed = WalkSpeed;
			// First press logic
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_ResetPressCount, this, &AOriginCharacter::ResetPressCount, 0.5f, false);
		}
		else if (PressedAmount >= 2)
		{
			// Second or further press logic within the timeframe
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
			PreviousSpeed = RunSpeed;
			// Optionally, reset the count immediately to start over
			ResetPressCount();
		}

	}
	else if (GetLocalRole() < ROLE_Authority)
	{
		ServerWisPressed();
	}


}

void AOriginCharacter::WisReleased()
{
	if (HasAuthority())
	{
		bWKey = false;
		GetCharacterMovement()->MaxWalkSpeed = 150;
	}
	else if (GetLocalRole() < ROLE_Authority)
	{
		ServerWisReleased();

	}
}

void AOriginCharacter::LeftShiftPressed()
{
	if (HasAuthority())
	{
		if (bWKey == true)
			GetCharacterMovement()->MaxWalkSpeed = 900;
	}
	else if (GetLocalRole() < ROLE_Authority)
	{
		ServerLeftShiftPressed();

	}
}

void AOriginCharacter::LeftShiftReleased()
{
	if (HasAuthority())
	{
		GetCharacterMovement()->MaxWalkSpeed = PreviousSpeed;
	}
	else if (GetLocalRole() < ROLE_Authority)
	{
		ServerLeftShiftReleased();
	}
}

void AOriginCharacter::OneKeyPressed()
{
	SwitchWeapon(0);
}

void AOriginCharacter::TwoKeyPressed()
{
	SwitchWeapon(1);
}



void AOriginCharacter::ServerWisPressed_Implementation()
{
	bWKey = true;
	PressedAmount++;
	if (PressedAmount == 1)
	{
		this->PreviousSpeed = WalkSpeed; // Assuming PreviousSpeed is a member variable
		// First press logic
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ResetPressCount, this, &AOriginCharacter::ResetPressCount, 0.5f, false);
	}
	else if (PressedAmount >= 2)
	{
		// Second or further press logic within the timeframe
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		this->PreviousSpeed = RunSpeed; // Update the member variable
		// Optionally, reset the count immediately to start over
		ResetPressCount();
	}
}

bool AOriginCharacter::ServerWisPressed_Validate()
{
	return true;
}

void AOriginCharacter::ServerWisReleased_Implementation()
{
	bWKey = false;
	GetCharacterMovement()->MaxWalkSpeed = 150;

}

bool AOriginCharacter::ServerWisReleased_Validate()
{
	return true;
}

void AOriginCharacter::ServerLeftCtrlPressed_Implementation()
{
	if (bGeneralStance == false && bScanStance == false && bCloseQuarters == false) bGeneralStance = true;

	if (bGeneralStance == true)
	{
		bGeneralStance = false;
		bScanStance = true;
		bCloseQuarters = false;
		WalkSpeed = 200;
		RunSpeed = 400;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		return;
	}

	if (bScanStance == true)
	{
		bGeneralStance = false;
		bScanStance = false;
		bCloseQuarters = true;
		WalkSpeed = 100;
		RunSpeed = 300;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		return;

	}

	if (bCloseQuarters == true)
	{
		bGeneralStance = true;
		bScanStance = false;
		bCloseQuarters = false;
		WalkSpeed = 300;
		RunSpeed = 600;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		return;
	}
	
}

bool AOriginCharacter::ServerLeftCtrlPressed_Validate()
{
	return true;
}


void AOriginCharacter::ServerLeftShiftPressed_Implementation()
{
	if (bWKey == true)
		GetCharacterMovement()->MaxWalkSpeed = 900;

}



bool AOriginCharacter::ServerLeftShiftPressed_Validate()
{
	return true;
}

void AOriginCharacter::ServerLeftShiftReleased_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = PreviousSpeed;


}

bool AOriginCharacter::ServerLeftShiftReleased_Validate()
{
	return true;
}


void AOriginCharacter::ServerSwitchWeapon_Implementation(int32 WeaponIndex)
{
	// Weapon switch logic goes here
	SwitchWeapon(WeaponIndex);
}

bool AOriginCharacter::ServerSwitchWeapon_Validate(int32 WeaponIndex)
{
	// Add any validation logic here
	// For now, we'll just return true to indicate it's always valid
	return true;
}