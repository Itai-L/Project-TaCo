// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
// #include "Logging/LogMacros.h"
// #include "Logging/Log.h"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Project_Taco/Weapon/Weapon.h" 
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "OriginCharacter.generated.h"



UCLASS()
class PROJECT_TACO_API AOriginCharacter : public ACharacter
{
	GENERATED_BODY()

	friend class UOriginAnimInstance;

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
	void OnRightMousePressed();
	void OnRightMouseReleased();
	void LeftCtrlPressed();
	void WisPressed();
	void WisReleased();
	void LeftShiftPressed();
	void LeftShiftReleased();
	void OneKeyPressed();
	void TwoKeyPressed();

	// Override GetLifetimeReplicatedProps to replicate bAiming
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void AttachWeaponToSocket(AWeapon* Weapon);

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TArray<TSubclassOf<AWeapon>> WeaponClasses;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TArray<AWeapon*> Weapons;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName WeaponSocketName;


private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere)
	bool bDoublePressW;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidger;

    int32 PressedAmount = 0;
    FTimerHandle TimerHandle_ResetPressCount;
	void ResetPressCount();
	


public:


	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SwitchWeapon(int32 WeaponIndex);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting", Replicated)
	bool bAiming;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stance", Replicated)
	bool bGeneralStance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stance", Replicated)
	bool bScanStance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stance", Replicated)
	bool bCloseQuarters;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",Replicated)
	bool bWKey;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", Replicated)
	float WalkSpeed = 300.0f; // Adjust as needed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", Replicated)
	float RunSpeed = 600.0f; // Adjust as needed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", Replicated)
	float PreviousSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", Replicated)
	int32 CurrentWeaponIndex;



	UFUNCTION(BlueprintCallable)
	bool isAiming();
	UFUNCTION(BlueprintCallable)
	bool isGeneralStance();
	UFUNCTION(BlueprintCallable)
	bool isScanStance();
	UFUNCTION(BlueprintCallable)
	bool isCloseQuarters();




	UFUNCTION(Server, Reliable, WithValidation)
	void ServerOnRightMouseReleased();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerOnRightMousePressed();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLeftCtrlPressed();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerWisPressed();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerWisReleased();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLeftShiftPressed();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLeftShiftReleased();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSwitchWeapon(int32 WeaponIndex);

};