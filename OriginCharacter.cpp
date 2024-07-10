// Fill out your copyright notice in the Description page of Project Settings.


#include "OriginCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Weapon/Weapon.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h" // Include this for timers
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h" // Included for replication
#include "Components/WidgetComponent.h"
#include "OriginAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"


AOriginCharacter::AOriginCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	//CameraBoom->SetupAttachment(GetMesh());
	//CameraBoom->TargetArmLength = 300.f;
	//CameraBoom->bUsePawnControlRotation = true;

	//FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	//FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	//FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;



	OverheadWidger = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidger->SetupAttachment(RootComponent);
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	
	//GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	/*GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);*/

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
	CurrentWeaponIndex = 0;
	bReplicates = true;
}

// Called when the game starts or when spawned
void AOriginCharacter::BeginPlay()
{
	Super::BeginPlay();
	//AttachCameraToHead();
	if (HasAuthority()) // Ensure this is only run on the server
	{
		CurrentWeaponIndex = -1;
	}

	SwitchWeapon(0);
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
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AOriginCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AOriginCharacter::FireButtonReleased);
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





void AOriginCharacter::AttachCameraToHead()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp)
	{
		CameraComponent->SetupAttachment(MeshComp);

		UE_LOG(LogTemp, Log, TEXT("Camera attached to head socket"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Mesh component not found"));
	}
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

void AOriginCharacter::AdjustCharacterMovement(float DeltaYaw)
{
	

	if (DeltaYaw > 90.0f)
	{
		
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bTurnRight = true;
		bTurnLeft = false;
	}
	else if (DeltaYaw < -90.0f)
	{
	
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bTurnRight = false;
		bTurnLeft = true;
	}
	else
	{
		
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bTurnRight = false;
		bTurnLeft = false;
	}
}

void AOriginCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AOriginCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AimOffset(DeltaTime);
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
		UE_LOG(LogTemp, Log, TEXT("Available Weapon Classes:"));
		for (int32 i = 0; i < WeaponClasses.Num(); ++i)
		{
			if (WeaponClasses[i])
			{
				UE_LOG(LogTemp, Log, TEXT("Weapon %d: %s"), i, *WeaponClasses[i]->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Weapon %d: None"), i);
			}
		}

		if (WeaponIndex >= 0 && WeaponIndex < WeaponClasses.Num())
		{
			for (AWeapon* Weapon : Weapons)
			{
				if (Weapon)
				{
					Weapon->Destroy();
				}
			}
			Weapons.Empty();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();
			AWeapon* NewWeapon = GetWorld()->SpawnActor<AWeapon>(WeaponClasses[WeaponIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (NewWeapon)
			{
				AttachWeaponToSocket(NewWeapon);
				Weapons.Add(NewWeapon);
				NewWeapon->SetOwningCharacter(this);

				LeftHandTransform = NewWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
				FVector OutPosition;
				FRotator OutRotation;
				GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
				LeftHandTransform.SetLocation(OutPosition);
				LeftHandTransform.SetRotation(FQuat(OutRotation));

				CurrentWeaponIndex = WeaponIndex;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to spawn weapon at index %d"), WeaponIndex);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid weapon index %d"), WeaponIndex);
		}
	}
	else
	{
		ServerSwitchWeapon(WeaponIndex);
	}
}



FTransform AOriginCharacter::getLeftHandTransform() {

	return LeftHandTransform;
}


//void AOriginCharacter::setAO_Yaw(float Yaw) {
//
//	AO_Yaw = Yaw;
//}

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
	DOREPLIFETIME(AOriginCharacter, Weapons);
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

//void AOriginCharacter::FireButtonPressed()
//{
//	bFiring = true;
//	PlayFireMontage();
//	ServerFire();
//	
//}
//void AOriginCharacter::FireButtonReleased()
//{
//	
//	bFiring = false;
//}
//
//void AOriginCharacter::ServerFire_Implementation()
//{
//	MulticastFire();
//}
//
//void AOriginCharacter::MulticastFire_Implementation()
//{
//	
//	PlayFireMontage();
//
//}
//
//void AOriginCharacter::PlayFireMontage()
//{
//	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
//
//	AnimInstance->Montage_Play(FireWeaponMontage);
//	Weapons[CurrentWeaponIndex]->Fire();
//
//
//}
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

bool AOriginCharacter::getTurnRight() {
	return bTurnRight;
}

bool AOriginCharacter::getTurnLeft() {
	return bTurnLeft;
}


	
void AOriginCharacter::AimOffset(float DeltaTime)
{
	
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f) // running, or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void AOriginCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AOriginCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

float AOriginCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void AOriginCharacter::OnRep_Weapons()
{
	UE_LOG(LogTemp, Log, TEXT("Weapons array replicated with %d items"), Weapons.Num());
	for (int32 i = 0; i < Weapons.Num(); ++i)
	{
		if (Weapons[i])
		{
			UE_LOG(LogTemp, Log, TEXT("Weapon %d: %s"), i, *Weapons[i]->GetName());
		}
	}
}

void AOriginCharacter::FireButtonPressed()
{
	bFiring = true;
	//PlayFireMontage();
	ServerFire();
}

void AOriginCharacter::FireButtonReleased()
{
	bFiring = false;
}

void AOriginCharacter::ServerFire_Implementation()
{
	MulticastFire();
}

bool AOriginCharacter::ServerFire_Validate()
{
	return true;
}

void AOriginCharacter::MulticastFire_Implementation()
{
	PlayFireMontage();
}

void AOriginCharacter::PlayFireMontage()
{
	UE_LOG(LogTemp, Log, TEXT("PlayFireMontage called"));

	// Log the animation montage and whether it's valid
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		UE_LOG(LogTemp, Log, TEXT("Playing fire weapon montage"));
		AnimInstance->Montage_Play(FireWeaponMontage);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimInstance or FireWeaponMontage is null"));
	}

	// Log the current weapon index
	UE_LOG(LogTemp, Log, TEXT("CurrentWeaponIndex: %d"), CurrentWeaponIndex);

	// Log the entire Weapons array
	UE_LOG(LogTemp, Log, TEXT("Weapons array contains %d elements"), Weapons.Num());
	for (int32 i = 0; i < Weapons.Num(); ++i)
	{
		if (Weapons[i])
		{
			UE_LOG(LogTemp, Log, TEXT("Weapon %d: %s"), i, *Weapons[i]->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Weapon %d: None"), i);
		}
	}

	// Check if the current weapon index is valid and log the weapon details
	if (Weapons.IsValidIndex(CurrentWeaponIndex))
	{
		if (Weapons[CurrentWeaponIndex])
		{
			UE_LOG(LogTemp, Log, TEXT("Firing weapon at index %d: %s"), CurrentWeaponIndex, *Weapons[CurrentWeaponIndex]->GetName());
			Weapons[CurrentWeaponIndex]->Fire();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Current weapon at index %d is null"), CurrentWeaponIndex);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid weapon index: %d"), CurrentWeaponIndex);
	}
}
