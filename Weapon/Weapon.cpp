#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"

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

void AWeapon::Fire()
{
    if (FireAnimation)
    {
        WeaponMesh->PlayAnimation(FireAnimation, false);
    }
}

void AWeapon::SetOwningCharacter(ACharacter* NewOwner)
{
    OwningCharacter = NewOwner;
    SetOwner(NewOwner);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWeapon, OwningCharacter);
}
