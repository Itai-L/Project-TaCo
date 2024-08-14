#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "../OriginCharacter.h"

AWeapon::AWeapon()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = false;

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(RootComponent);
    WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetRootComponent(WeaponMesh);

    AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
    AreaSphere->SetupAttachment(RootComponent);
    AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

   
}

void AWeapon::Fire(const FVector& HitTarget)
{
    if (FireAnimation)
    {
        WeaponMesh->PlayAnimation(FireAnimation, false);
    }

    if (CasingClass)
    {
        const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("ShellEjectionSocket"));
        if (AmmoEjectSocket)
        {
            FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);

            UWorld* World = GetWorld();
            if (World)
            {
                World->SpawnActor<ACasing>(
                    CasingClass,
                    SocketTransform.GetLocation(),
                    SocketTransform.GetRotation().Rotator()
                );
            }
        }
    }
}

void AWeapon::JamAnimaton()
{
    if (OriginCharacter == nullptr)
    {
        OriginCharacter = Cast<AOriginCharacter>(OwningCharacter);

        if (!OriginCharacter)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to set OriginCharacter from OwningCharacter."));
            return;
        }
    }

    // Update jam states based on the character's state
    if (OriginCharacter)
    {
        bFirstJam = OriginCharacter->GetFirstJam();
        bSecondJam = OriginCharacter->GetSecondJam();
        bThirdJam = OriginCharacter->GetThirdJam();


    }
    if (bFirstJam && FirstJamAnimation)
    {
        WeaponMesh->PlayAnimation(FirstJamAnimation, false);
    }
    else if (bSecondJam && SecondJamAnimation)
    {
        WeaponMesh->PlayAnimation(SecondJamAnimation, false);
    }
    else if (bThirdJam && ThirdJamAnimation)
    {
        WeaponMesh->PlayAnimation(ThirdJamAnimation, false);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load or play jam animation."));
    }
}

void AWeapon::SetOwningCharacter(ACharacter* NewOwner)
{
    OwningCharacter = NewOwner;
    SetOwner(NewOwner);

    OriginCharacter = Cast<AOriginCharacter>(NewOwner);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWeapon, OwningCharacter);
}
