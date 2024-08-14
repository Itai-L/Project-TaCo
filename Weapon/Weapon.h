// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState
{
    EWS_Initial UMETA(DisplayName = "Initial Name"),
    EWS_Equipped UMETA(DisplayName = "Equipped"),
    EWS_Dropped UMETA(DisplayName = "Dropped"),
};

UCLASS()
class PROJECT_TACO_API AWeapon : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AWeapon();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Fire weapon method
    virtual void Fire(const FVector& HitTarget);

    UFUNCTION()
    void JamAnimaton();

    // Sets the owning character
    void SetOwningCharacter(ACharacter* NewOwner);

    // Getter for Weapon Mesh
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

private:
    // Weapon mesh component
    UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
    USkeletalMeshComponent* WeaponMesh;

    // Collision component for area detection
    UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
    class USphereComponent* AreaSphere;

    // Current weapon state
    UPROPERTY(VisibleAnywhere)
    EWeaponState WeaponState;

    // Reference to the owning character
    UPROPERTY(Replicated)
    ACharacter* OwningCharacter;

    // Reference to the specific character class (AOriginCharacter)
    UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
    class AOriginCharacter* OriginCharacter;

    // Animation assets
    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    UAnimationAsset* FirstJamAnimation;

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    UAnimationAsset* SecondJamAnimation;

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
   UAnimationAsset* ThirdJamAnimation;

    UPROPERTY(EditAnywhere, Category = "Weapon Properties")
    class UAnimationAsset* FireAnimation;


    // Casing class for spawning shell casings
    UPROPERTY(EditAnywhere)
    TSubclassOf<class ACasing> CasingClass;

    UPROPERTY(VisibleAnywhere, Category = "Jams")
    bool bFirstJam;

    UPROPERTY(VisibleAnywhere, Category = "Jams")
    bool bSecondJam;

    UPROPERTY(VisibleAnywhere, Category = "Jams")
    bool bThirdJam;
};
