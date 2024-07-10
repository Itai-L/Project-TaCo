// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState 
{
EWS_Initial UMETA(DisplayName="Initial Name"),
EWS_Equpiped UMETA(DisplayName = "Equipped"),
EWS_Dropped UMETA(DisplayName = " Dropped"),

};


UCLASS()
class PROJECT_TACO_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	void Fire();
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")

	class UAnimationAsset* FireAnimation;

	void SetOwningCharacter(ACharacter* NewOwner); // Method declaration
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private:
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere)
	EWeaponState WeaponState;

	UPROPERTY(Replicated)
	ACharacter* OwningCharacter;

};
