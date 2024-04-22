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

	// CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	// CameraBoom->SetupAttachment(GetMesh());
	// CameraBoom->TargetArmLength = 600.f;
	// CameraBoom->bUsePawnControlRotation = true;

	//FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	////FollowCamera->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "head");
	// FollowCamera->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("head"));
	//FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	//FollowCamera->bUsePawnControlRotation = false;

	// Assuming this code is within the constructor of your character class

// Assuming this code is within the constructor of your character class

/// Create the follow camera
	




	//UseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidger = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidger->SetupAttachment(RootComponent);
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	;

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
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AOriginCharacter::CrouchPressed);
	PlayerInputComponent->BindAction("Prone", IE_Released, this, &AOriginCharacter::PronePressed);

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
	DOREPLIFETIME(AOriginCharacter, bInProne);
	



}


void AOriginCharacter::OnRightMousePressed()
{
	if (HasAuthority())
	{
	
		bAiming = true;

		
			GetCharacterMovement()->MaxWalkSpeed = 150;

		UE_LOG(LogTemp, Warning, TEXT("The value of MaxWalkSpeed is %f."), GetCharacterMovement()->MaxWalkSpeed);


	}
	else if (GetLocalRole() < ROLE_Authority)
	{
		ServerOnRightMousePressed();
	}
}

void AOriginCharacter::ServerOnRightMousePressed_Implementation()
{
	
	bAiming = true;
	if (GetCharacterMovement()->MaxWalkSpeed > 300)
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

bool AOriginCharacter::ServerOnRightMousePressed_Validate()
{
	return true;
}


void AOriginCharacter::OnRightMouseReleased()
{
	if (HasAuthority())
	{
		bAiming = false;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	
	}
	else if (GetLocalRole() < ROLE_Authority)
	{
		
		ServerOnRightMouseReleased();
	}
}

void AOriginCharacter::ServerOnRightMouseReleased_Implementation()
{
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
			RunSpeed = 600;
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			return;
		}

		if (bScanStance == true)
		{
			bGeneralStance = false;
			bScanStance = false;
			bCloseQuarters = true;
			WalkSpeed = 150;
			RunSpeed = 600;
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

void AOriginCharacter::ServerLeftCtrlPressed_Implementation()
{
	if (bGeneralStance == false && bScanStance == false && bCloseQuarters == false) bGeneralStance = true;

	if (bGeneralStance == true)
	{
		bGeneralStance = false;
		bScanStance = true;
		bCloseQuarters = false;
		WalkSpeed = 200;
		RunSpeed = 600;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		return;
	}

	if (bScanStance == true)
	{
		bGeneralStance = false;
		bScanStance = false;
		bCloseQuarters = true;
		WalkSpeed = 150;
		RunSpeed = 600;
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
			if (bIsCrouched)
				GetCharacterMovement()->MaxWalkSpeed = 100;
			if (bInProne) {
				GetCharacterMovement()->MaxWalkSpeed = 50;
				if (bAiming )
					OnRightMouseReleased();
			}

			GetWorld()->GetTimerManager().SetTimer(TimerHandle_ResetPressCount, this, &AOriginCharacter::ResetPressCount, 0.5f, false);
		}
		else if (PressedAmount >= 2)
		{
			if (bIsCrouched)
				UnCrouch();
			if (bInProne)
				bInProne = false;
			if (bAiming)
				OnRightMouseReleased();
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

void AOriginCharacter::ServerWisPressed_Implementation()
{
	bWKey = true;
	PressedAmount++;
	if (PressedAmount == 1)
	{
		PreviousSpeed = WalkSpeed;
		// First press logic
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		if (bIsCrouched)
			GetCharacterMovement()->MaxWalkSpeed = 100;
		if (bInProne) {
			GetCharacterMovement()->MaxWalkSpeed = 50;
			if (bAiming)
				OnRightMouseReleased();
		}

		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ResetPressCount, this, &AOriginCharacter::ResetPressCount, 0.5f, false);
	}
	else if (PressedAmount >= 2)
	{
		if (bIsCrouched)
			UnCrouch();
		if (bInProne)
			bInProne = false;
		if (bAiming)
			OnRightMouseReleased();
		// Second or further press logic within the timeframe
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		PreviousSpeed = RunSpeed;
		// Optionally, reset the count immediately to start over
		ResetPressCount();
	}
}

bool AOriginCharacter::ServerWisPressed_Validate()
{
	return true;
}


void AOriginCharacter::WisReleased()
{
	if (HasAuthority())
	{
		bWKey = false;
		GetCharacterMovement()->MaxWalkSpeed = 150;
		if (Crouched)
		{
			GetCharacterMovement()->MaxWalkSpeed = 100;
			Crouch();
		}
		if (bProneState)
		{
			GetCharacterMovement()->MaxWalkSpeed = 50;
			bInProne = true;
		}


	}




	else if (GetLocalRole() < ROLE_Authority)
	{
		ServerWisReleased();

	}
}

void AOriginCharacter::ServerWisReleased_Implementation()
{
	bWKey = false;
	GetCharacterMovement()->MaxWalkSpeed = 150;
	if (Crouched)
	{
		GetCharacterMovement()->MaxWalkSpeed = 100;
		Crouch();
	}
	if (bProneState)
	{
		GetCharacterMovement()->MaxWalkSpeed = 50;
		bInProne = true;
	}

}

bool AOriginCharacter::ServerWisReleased_Validate()
{
	return true;
}

void AOriginCharacter::LeftShiftPressed()
{
	if (HasAuthority())
	{
		if (bAiming == true)
			OnRightMouseReleased();

		if (Crouched)
			UnCrouch();
	
		if (bInProne)
			bInProne = false;

		if (bWKey == true)
			GetCharacterMovement()->MaxWalkSpeed = 900;



	}



	else if (GetLocalRole() < ROLE_Authority)
	{
		ServerLeftShiftPressed();

	}
}

void AOriginCharacter::ServerLeftShiftPressed_Implementation()
{

	if (bAiming == true)
		OnRightMouseReleased();

	if (Crouched)
		UnCrouch();

	if (bInProne)
		bInProne = false;

	if (bWKey == true)
		GetCharacterMovement()->MaxWalkSpeed = 900;

}

bool AOriginCharacter::ServerLeftShiftPressed_Validate()
{
	return true;
}

void AOriginCharacter::LeftShiftReleased()
{
	if (HasAuthority())
	{
		GetCharacterMovement()->MaxWalkSpeed = PreviousSpeed;
		if (Crouched)
		{
			GetCharacterMovement()->MaxWalkSpeed = 100;
			Crouch();
		}

		if (bProneState)
		{
			GetCharacterMovement()->MaxWalkSpeed = 50;
			bInProne = true;
		}



	}
	else if (GetLocalRole() < ROLE_Authority)
	{
		ServerLeftShiftReleased();
	}
}

void AOriginCharacter::ServerLeftShiftReleased_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = PreviousSpeed;
	if (Crouched)
	{
		GetCharacterMovement()->MaxWalkSpeed = 100;
		Crouch();
	}

	if (bProneState)
	{
		GetCharacterMovement()->MaxWalkSpeed = 50;
		bInProne = true;
	}


}

bool AOriginCharacter::ServerLeftShiftReleased_Validate()
{
	return true;
}



void AOriginCharacter::OneKeyPressed()
{
	SwitchWeapon(0);
}

void AOriginCharacter::TwoKeyPressed()
{
	SwitchWeapon(1);
}

void AOriginCharacter::CrouchPressed()
{
	if (!bIsCrouched)
	{
		if (bInProne)
		{
			bInProne = false;
			bProneState = false;
		
		}

		GetCharacterMovement()->MaxWalkSpeed = 100;
		Crouch();
		bIsCrouched = true;
	}
	else
	{
		UnCrouch();
		bIsCrouched = false;

		if (bGeneralStance)
			WalkSpeed = 300;
		else if (bScanStance)
			WalkSpeed = 200;
		else if (bCloseQuarters)
			WalkSpeed = 150;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}

	
}



void AOriginCharacter::ServerCrouchPressed_Implementation()
{
	
		if (!bIsCrouched)
		{
			GetCharacterMovement()->MaxWalkSpeed = 100;
			Crouch();
			bIsCrouched = true;
			Crouched = true;

		}
		else
		{
			UnCrouch();
			bIsCrouched = false;
			Crouched = false;
			if (bGeneralStance)
				WalkSpeed = 300;
			else if (bScanStance)
				WalkSpeed = 200;
			else if (bCloseQuarters)
				WalkSpeed = 150;
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

		}
	

}

bool AOriginCharacter::ServerCrouchPressed_Validate()
{
	return true;
}




void AOriginCharacter::PronePressed()
{

	if (HasAuthority())
	{
		if (!bInProne)
		{

			if (bIsCrouched)
			{
				bIsCrouched = false;
				UnCrouch();
				
			}

			bInProne = true;
			bProneState = true;
			GetCharacterMovement()->MaxWalkSpeed = 50;
			
		}
		else
		{
			bInProne = false;
			bProneState = false;

			if (bGeneralStance)
				WalkSpeed = 300;
			else if (bScanStance)
				WalkSpeed = 200;
			else if (bCloseQuarters)
				WalkSpeed = 150;
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		}

		
	}


	else if (GetLocalRole ()< ROLE_Authority)
	{

		ServerPronePressed();
	}

	
}



void AOriginCharacter::ServerPronePressed_Implementation()
{


	if (!bInProne)
	{

		/*if (bIsCrouched)
		{
			bIsCrouched = false;
			UnCrouch();

		}*/

		bInProne = true;
		bProneState = true;
		GetCharacterMovement()->MaxWalkSpeed = 50;

	}
	else
	{
		bInProne = false;
		bProneState = false;

		if (bGeneralStance)
			WalkSpeed = 300;
		else if (bScanStance)
			WalkSpeed = 200;
		else if (bCloseQuarters)
			WalkSpeed = 150;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

bool AOriginCharacter::ServerPronePressed_Validate()
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