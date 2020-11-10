// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));

	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	MuzzleLocation->SetupAttachment(WeaponMesh);
	MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::Fire()
{
}

FHitResult AWeapon::TraceShot(const FVector& TraceOrigin, const FVector& TraceDirection) const
{

	// Perform trace to retrieve hit info
	//FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, Instigator);
	FCollisionQueryParams TraceParams;
	TraceParams.bReturnPhysicalMaterial = true;
	TraceParams.AddIgnoredActor(Instigator);

	float WeaponRange = 100000.0f;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceOrigin, TraceOrigin + TraceDirection * WeaponRange, ECC_Visibility, TraceParams);

	DrawDebugLine(GetWorld(), TraceOrigin, TraceOrigin + TraceDirection * WeaponRange, FColor::Green, false, 2.0f, 0, 1.f);

	return Hit;
}

void AWeapon::SetOwnerCharacter(APawn* NewOwner)
{
	Instigator = NewOwner;
	// net owner for RPC calls
	//SetOwner(NewOwner);
}
