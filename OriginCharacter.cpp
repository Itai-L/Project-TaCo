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
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Project_Taco/Project_Taco.h"
#include "Project_Taco/PlayerController/OriginPlayerController.h"
#include "Project_Taco/GameModes/OriginGameMode.h"


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
//	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;


	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

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

	bGeneralStance = true;
	//AttachCameraToHead();
	if (HasAuthority()) // Ensure this is only run on the server
	{
		CurrentWeaponIndex = -1;
	}

	SwitchWeapon(0);
	SprintingTimeCounter = 0.0f;
	RunningTimeCounter = 0.0f;

	Ammunition = 150;
	Magazine = 30;
	Canfire = true;

	OriginPlayerController = Cast<AOriginPlayerController>(Controller);
	if (OriginPlayerController)
	{
		OriginPlayerController->SetHUDHealth(Health, MaxHealth);
		OriginPlayerController->SetHUDFatigue(Fatigue);

	}


	UpdateHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AOriginCharacter::ReceiveDamage);
	}



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
	PlayerInputComponent->BindAction("ThreeKey", IE_Released, this, &AOriginCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AOriginCharacter::CrouchPressed);
	PlayerInputComponent->BindAction("Prone", IE_Released, this, &AOriginCharacter::PronePressed);
	PlayerInputComponent->BindAction("RButton", IE_Pressed, this, &AOriginCharacter::RButtonPressed);
	PlayerInputComponent->BindAction("RButton", IE_Released, this, &AOriginCharacter::RButtonReleased);

}


TArray<FString> MontageSections = { TEXT("Death1"), TEXT("Death2"), TEXT("Death3"), TEXT("DeathAim1"),TEXT("DeathAim2"),TEXT("ProneDeath1"),TEXT("ProneDeath2"),};

// Function to get a random montage section
FName GetRandomMontageSection(bool bAiming, bool bInProne)
{
    int32 RandomIndex;

    if (bInProne)
    {
        RandomIndex = UKismetMathLibrary::RandomIntegerInRange(5, 6);
		return FName(*MontageSections[RandomIndex]);
    }
    else if (bAiming)
    {
        RandomIndex = UKismetMathLibrary::RandomIntegerInRange(3, 4);
		return FName(*MontageSections[RandomIndex]);

    }
    else
    {
        RandomIndex = UKismetMathLibrary::RandomIntegerInRange(0, 2);
		return FName(*MontageSections[RandomIndex]);

    }
	return FName(*MontageSections[RandomIndex]);

    
}


void AOriginCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{

		FName RandomSection = GetRandomMontageSection(bAiming, bInProne);
		AnimInstance->Montage_Play(ElimMontage);
		AnimInstance->Montage_JumpToSection(RandomSection, ElimMontage);
	}
}

void AOriginCharacter::PlayJamsMontage(FName Section)
{
	if (HasAuthority())
	{
		MulticastPlayJamsMontage(Section);
	}
	else if (GetLocalRole() < ROLE_Authority)
	{
		ServerPlayJamsMontage(Section);
	}
}

void AOriginCharacter::ServerPlayJamsMontage_Implementation(FName Section)
{
	MulticastPlayJamsMontage(Section); // Ensure that all clients see the montage
}

void AOriginCharacter::MulticastPlayJamsMontage_Implementation(FName Section)
{
	FName JamSection = Section;
	if (!JamsMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("JamsMontage is not assigned!"));
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimInstance is null!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Playing Montage: %s"), *JamsMontage->GetName());

	// Stop any currently playing montage before starting a new one
	AnimInstance->Montage_Stop(0.2f, JamsMontage);
	AnimInstance->Montage_Play(JamsMontage);

	// Attempt to jump to the specified section
	AnimInstance->Montage_JumpToSection(JamSection, JamsMontage);

	// Log the section being jumped to
	UE_LOG(LogTemp, Warning, TEXT("Jumped to Section: %s"), *JamSection.ToString());
}


//void AOriginCharacter::PlayElimMontage()
//{
//	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
//	if (AnimInstance && ElimMontage)
//	{
//		AnimInstance->Montage_Play(ElimMontage);
//	}
//}

void AOriginCharacter::Elim_Implementation()
{
	bElimmed = true;
	PlayElimMontage();
	// Disable character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (OriginPlayerController)
	{
		DisableInput(OriginPlayerController);
	}
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AOriginCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		AOriginGameMode* OriginGameMode = GetWorld()->GetAuthGameMode<AOriginGameMode>();
		if (OriginGameMode)
		{
			OriginPlayerController = OriginPlayerController == nullptr ? Cast<AOriginPlayerController>(Controller) : OriginPlayerController;
			AOriginPlayerController* AttackerController = Cast<AOriginPlayerController>(InstigatorController);
			OriginGameMode->PlayerEliminated(this, OriginPlayerController, AttackerController);
		}
	}

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
	IncrementFatigue(DeltaTime);
	if(!Canfire){ Jams(); }
	
	

	if (bIsSprinting) {
		bIsRunning = false;
		SprintingTimeCounter += DeltaTime;
		//_LOG(LogTemp, Warning, TEXT("Sprinting Time: %f seconds"), SprintingTimeCounter);
		AdjustSpeed(900.f);
	}
	else {
		SprintRecoveryTime += SprintingTimeCounter;
		SprintingTimeCounter = 0.f;
	}

	if (bIsRunning) {
		bIsSprinting = false;
		RunningTimeCounter += DeltaTime;
		///LOG(LogTemp, Warning, TEXT("Running Time: %f seconds"), RunningTimeCounter);
		AdjustSpeed(600.f);
	}
	else {
		RunningRecoveryTime += RunningTimeCounter;
		RunningTimeCounter = 0.f;
	}

	// Only decrease recovery time if the player is neither running nor sprinting
	if (!bIsRunning && !bIsSprinting)
	{
		DecreaseRecoveryTime(DeltaTime);
	}

	// Calculate total recovery time
	TotalRecoveryTime = SprintRecoveryTime + RunningRecoveryTime;
	//UE_LOG(LogTemp, Warning, TEXT("Total Recovery Time: %f seconds"), TotalRecoveryTime);

	UpdateHUDFatigue();
}

void AOriginCharacter::HandleJamManuInput(int Input) {

	if (Input == 1)
	{
		PlayJamsMontage("FirstJam");

		if (bFirstJam)
		{
			bFirstJam = false;
			Canfire = true;
		}
		Magazine--;

	}
	else if (Input == 2)
	{
		PlayJamsMontage("SecondJam");
		if (bSecondJam)
		{
			bSecondJam = false;
			Canfire = true;
		}
		Magazine--;
	}
	else if (Input == 3)
	{
	
		PlayJamsMontage("ThirdJam");

		if (Ammunition > 0)
		{
			Ammunition -= 30;
			Magazine = 30;
			Canfire = true;
		}
		else
		{
			bThirdJam = false;
			bFirstJam = false;
			Canfire = false;
			UE_LOG(LogTemp, Warning, TEXT("Out of Ammo"));


		}
	}
	
}





void AOriginCharacter::DecreaseRecoveryTime(float DeltaTime)
{
	// Decrease Running Recovery Time
	if (RunningRecoveryTime > 0.f) {
		RunningRecoveryTime -= DeltaTime; // Decreases by 1 second each second
		RunningRecoveryTime = FMath::Max(RunningRecoveryTime, 0.f); // Ensure it doesn't go below 0
	}

	// Decrease Sprinting Recovery Time
	if (SprintRecoveryTime > 0.f) {
		SprintRecoveryTime -= DeltaTime / 2.f; // Decreases by 1 second every 2 seconds
		SprintRecoveryTime = FMath::Max(SprintRecoveryTime, 0.f); // Ensure it doesn't go below 0
	}
}

void AOriginCharacter::IncrementFatigue(float DeltaTime)
{

	float CurrentSpeed = GetVelocity().Size();
	//UE_LOG(LogTemp, Warning, TEXT("Current Speed: %f"), CurrentSpeed);

	float FatigueIncrement = 0.0f;

	if (bIsSprinting)
	{
		FatigueIncrement = 0.075f / 60.0f; // Base increment for sprinting
		//UE_LOG(LogTemp, Warning, TEXT("Sprinting: Base Fatigue increment %f. Current Fatigue: %f"), FatigueIncrement * DeltaTime * 60.0f, Fatigue);
	}
	else if (bIsRunning)
	{
		FatigueIncrement = 0.0583f / 60.0f; // Base increment for running
		//UE_LOG(LogTemp, Warning, TEXT("Running: Base Fatigue increment %f. Current Fatigue: %f"), FatigueIncrement * DeltaTime * 60.0f, Fatigue);
	}
	else if (bAiming)
	{
		FatigueIncrement = (CurrentSpeed == 0 ? 0.0583f / 60.0f : 0.167f / 60.0f); // Adjusted for speed
		//UE_LOG(LogTemp, Warning, TEXT("Aiming: Base Fatigue increment %f. Current Fatigue: %f"), FatigueIncrement * DeltaTime * 60.0f, Fatigue);
	}
	else if (bIsCrouched)
	{
		FatigueIncrement = (CurrentSpeed == 0 ? 0.0167f / 60.0f : 0.0833f / 60.0f); // Adjusted for speed
		//UE_LOG(LogTemp, Warning, TEXT("Crouching: Base Fatigue increment %f. Current Fatigue: %f"), FatigueIncrement * DeltaTime * 60.0f, Fatigue);
	}
	else if (bInProne)
	{
		FatigueIncrement = (CurrentSpeed == 0 ? 0.0167f / 60.0f : 0.066f / 60.0f); // Adjusted for speed
		//UE_LOG(LogTemp, Warning, TEXT("Prone: Base Fatigue increment %f. Current Fatigue: %f"), FatigueIncrement * DeltaTime * 60.0f, Fatigue);
	}
	else if (bGeneralStance)
	{
		FatigueIncrement = 0.0167f / 60.0f; // General stance increment
		//UE_LOG(LogTemp, Warning, TEXT("General Stance: Base Fatigue increment %f. Current Fatigue: %f"), FatigueIncrement * DeltaTime * 60.0f, Fatigue);
	}
	else if (bScanStance)
	{
		FatigueIncrement = 0.025f / 60.0f; // Scan stance increment
		//UE_LOG(LogTemp, Warning, TEXT("Scan Stance: Base Fatigue increment %f. Current Fatigue: %f"), FatigueIncrement * DeltaTime * 60.0f, Fatigue);
	}
	else if (bCloseQuarters)
	{
		FatigueIncrement = 0.0416f / 60.0f; // Close quarters stance increment
		//UE_LOG(LogTemp, Warning, TEXT("Close Quarters: Base Fatigue increment %f. Current Fatigue: %f"), FatigueIncrement * DeltaTime * 60.0f, Fatigue);
	}

	Fatigue += FatigueIncrement * DeltaTime * 60.0f;

	// Ensure Fatigue doesn't exceed the max value
	Fatigue = FMath::Clamp(Fatigue, 0.0f, 100.f);

	//UE_LOG(LogTemp, Warning, TEXT("Current Fatigue after update: %f"), Fatigue);


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

USkeletalMeshComponent* AOriginCharacter::GetCurrentWeaponMesh() const
{
	if (Weapons.IsValidIndex(CurrentWeaponIndex) && Weapons[CurrentWeaponIndex])
	{
		return Weapons[CurrentWeaponIndex]->GetWeaponMesh();
	}
	return nullptr;
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

void AOriginCharacter::RButtonPressed()
{
	if (HasAuthority())
		JamsMenu = true;
	else if (GetLocalRole() < ROLE_Authority)
		ServerRButtonPressed();
	

}

void AOriginCharacter::ServerRButtonPressed_Implementation()
{

	JamsMenu = true;
}




void AOriginCharacter::RButtonReleased()
{
	if (HasAuthority())
		JamsMenu = false;
	else if (GetLocalRole() < ROLE_Authority)
		ServerRButtonReleased();

}

void AOriginCharacter::ServerRButtonReleased_Implementation()
{

	JamsMenu = false;
}

bool AOriginCharacter::ServerRButtonReleased_Validate()
{

	return true;
}


bool AOriginCharacter::ServerRButtonPressed_Validate()
{

	return true;
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
	DOREPLIFETIME(AOriginCharacter, Health);
	DOREPLIFETIME(AOriginCharacter, JamsMenu);

}

//void AOriginCharacter::TraceUnderCrosshairs(FHitResult& TraceHitResult)
//{
//	FVector2D ViewportSize;
//	if (GEngine && GEngine->GameViewport)
//	{
//		GEngine->GameViewport->GetViewportSize(ViewportSize);
//	}
//
//	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
//	FVector CrosshairWorldPosition;
//	FVector CrosshairWorldDirection;
//	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
//		UGameplayStatics::GetPlayerController(this, 0),
//		CrosshairLocation,
//		CrosshairWorldPosition,
//		CrosshairWorldDirection
//	);
//
//	if (bScreenToWorld)
//	{
//		FVector Start = CrosshairWorldPosition;
//
//		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
//
//		GetWorld()->LineTraceSingleByChannel(
//			TraceHitResult,
//			Start,
//			End,
//			ECollisionChannel::ECC_Visibility
//		);
//	}
//}


//switch (PlayerStance)
		//{
		//case 1:
		//	Fatigue += 0.0167f;
		//	break;
		//case 2:
		//	Fatigue += 0.0333f;
		//	break;
		//case 3:
		//	Fatigue += 0.05f;
		//	break;
		//default:
		//	break;
		//}
	

void AOriginCharacter::AdjustSpeed(float InitSpeed)
{
	// Calculate the target speed based on fatigue
	float FatigueFactor = FMath::Clamp(1.0f - (Fatigue / 100.0f), 0.5f, 1.0f);
	float TargetSpeed = InitSpeed * FatigueFactor;

	// Gradually interpolate to the target speed
	float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;
	float InterpolatedSpeed;
	if (bIsSprinting) {
		InterpolatedSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, GetWorld()->GetDeltaSeconds(), 3.0f); // 2.0f is the speed of interpolation, adjust as needed
	}
	else if (bIsRunning){
		InterpolatedSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, GetWorld()->GetDeltaSeconds(), 1.0f);
		}


	// Apply the interpolated speed
	GetCharacterMovement()->MaxWalkSpeed = InterpolatedSpeed;

	UE_LOG(LogTemp, Warning, TEXT("Sprinting: Interpolated Max Walk Speed: %f due to Fatigue: %f"), InterpolatedSpeed, Fatigue);
}


void AOriginCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();

}


void AOriginCharacter::UpdateHUDHealth()
{
	OriginPlayerController = OriginPlayerController == nullptr ? Cast<AOriginPlayerController>(Controller) : OriginPlayerController;
	if (OriginPlayerController)
	{
		OriginPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}


void AOriginCharacter::UpdateHUDFatigue()
{
	OriginPlayerController = OriginPlayerController == nullptr ? Cast<AOriginPlayerController>(Controller) : OriginPlayerController;
	if (OriginPlayerController)
	{
		OriginPlayerController->SetHUDFatigue(Fatigue);
	}
}

void AOriginCharacter::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	USkeletalMeshComponent* CurrentWeaponMesh = GetCurrentWeaponMesh();
	if (CurrentWeaponMesh)
	{
		// Log that we have the current weapon mesh
		UE_LOG(LogTemp, Log, TEXT("CurrentWeaponMesh is valid"));

		// Get the muzzle socket location and rotation
		FVector MuzzleLocation = CurrentWeaponMesh->GetSocketLocation(FName("MuzzleFlashSocket"));
		FRotator MuzzleRotation = CurrentWeaponMesh->GetSocketRotation(FName("MuzzleFlashSocket"));

		// Log the muzzle location and rotation
		UE_LOG(LogTemp, Log, TEXT("MuzzleLocation: %s"), *MuzzleLocation.ToString());
		UE_LOG(LogTemp, Log, TEXT("MuzzleRotation: %s"), *MuzzleRotation.ToString());

		// Set CrosshairWorldPosition to the MuzzleLocation
		FVector CrosshairWorldPosition = MuzzleLocation;
		FVector CrosshairWorldDirection = MuzzleRotation.Vector();

		// Log the crosshair world position and direction
		UE_LOG(LogTemp, Log, TEXT("CrosshairWorldPosition: %s"), *CrosshairWorldPosition.ToString());
		UE_LOG(LogTemp, Log, TEXT("CrosshairWorldDirection: %s"), *CrosshairWorldDirection.ToString());

		// Perform the line trace
		FVector Start = CrosshairWorldPosition;
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		bool bHit = GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		// Log the trace result
		if (bHit && TraceHitResult.GetActor())
		{
			UE_LOG(LogTemp, Log, TEXT("Line trace hit: %s"), *TraceHitResult.GetActor()->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Line trace did not hit anything"));
		}
	}
	else
	{
		// Log that the current weapon mesh is null
		UE_LOG(LogTemp, Error, TEXT("CurrentWeaponMesh is null"));
	}
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
			bIsRunning = true;
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

		bIsRunning = false;

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
			GetCharacterMovement()->MaxWalkSpeed = 900; // Reset to initial sprint speed

		bIsSprinting = true;
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

		bIsSprinting = false;

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
	if (JamsMenu)
	{
		//JamResolveInput = 1;
		HandleJamManuInput(1);
		return;
	}
	else { SwitchWeapon(0);}

}

void AOriginCharacter::TwoKeyPressed()
{
	if (JamsMenu)
	{
		
		HandleJamManuInput(2);
		return;
	}else{ SwitchWeapon(1); }
	
	

}

void AOriginCharacter::ThreeKeyPressed()
{
	if (JamsMenu)
	{
		HandleJamManuInput(3);
		return;
	}

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
	if (!Canfire)
		return;

	bFiring = true;
	Magazine--;

	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult);
	ServerFire(HitResult.ImpactPoint);

	// Check for First Jam with 1% chance
	float FirstJamChance = FMath::FRand();
	if (FirstJamChance <= 0.01f) // 1% chance
	{
		bFirstJam = true;
		UE_LOG(LogTemp, Warning, TEXT("First Jam Triggered!"));
		Canfire = false;
	}

	// Check for Second Jam with increasing chance
	float SecondJamRoll = FMath::FRand();
	if (SecondJamRoll <= SecondJamChance) // Current chance
	{
		bSecondJam = true;
		UE_LOG(LogTemp, Warning, TEXT("Second Jam Triggered!"));
		SecondJamChance = 0.001f; // Reset chance after it triggers
		Canfire = false;
	}
	else
	{
		// Increment the chance for next time
		SecondJamChance += 0.001f; // Increase by 0.1% (0.001)
		UE_LOG(LogTemp, Warning, TEXT("Second Jam Chance: %f"), SecondJamChance);
	}

	if (Magazine == 0)
	{
		Canfire = false;
		bThirdJam = true;
	}

	UE_LOG(LogTemp, Error, TEXT("Magazine Count: %d"), Magazine);
}


void AOriginCharacter::FireButtonReleased()
{
	bFiring = false;
}

void AOriginCharacter::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

//bool AOriginCharacter::ServerFire_Validate()
//{
//	return true;
//}

void AOriginCharacter::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	PlayFireMontage(TraceHitTarget);
}

void AOriginCharacter::PlayFireMontage(const FVector_NetQuantize& TraceHitTarget)
{


	// Log the animation montage and whether it's valid
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimInstance or FireWeaponMontage is null"));
	}

	

	// Check if the current weapon index is valid and log the weapon details
	if (Weapons.IsValidIndex(CurrentWeaponIndex))
	{
		if (Weapons[CurrentWeaponIndex])
		{

			Weapons[CurrentWeaponIndex]->Fire(TraceHitTarget);
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

void AOriginCharacter::Jams()
{
	if (bFirstJam || bSecondJam || bThirdJam)
	{
		if (Weapons.IsValidIndex(CurrentWeaponIndex))
		{
			if (Weapons[CurrentWeaponIndex])
			{

				Weapons[CurrentWeaponIndex]->JamAnimaton();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Current weapon at index %d is null"), CurrentWeaponIndex);
			}
		}

	}



}

void AOriginCharacter::PlayHitReactMontage()
{
	UE_LOG(LogTemp, Warning, TEXT("PlayHitReactMontage called."));

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimInstance found."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimInstance not found."));
	}

	if (AnimInstance && HitReactMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("Playing HitReactMontage."));
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		UE_LOG(LogTemp, Warning, TEXT("Jumping to section: %s"), *SectionName.ToString());
		AnimInstance->Montage_JumpToSection(SectionName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot play HitReactMontage: Either AnimInstance or HitReactMontage is null."));
	}
}




