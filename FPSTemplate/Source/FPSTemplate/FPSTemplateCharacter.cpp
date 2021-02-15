// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "FPSTemplateCharacter.h"
#include "FPSTemplateProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "CustomCharacterMovementComponent.h"
#include "RollbackDebugComponent.h"
#include "ShapeManagerComponent.h"
#include "Misc/CString.h"
#include "Misc/DateTime.h"
#include "Engine/NetDriver.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AFPSTemplateCharacter

AFPSTemplateCharacter::AFPSTemplateCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCustomCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Operations on the third-person mesh
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->SetOnlyOwnerSee(false);

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	UE_LOG(LogGauntlet, Display, TEXT("Setup player!"));

	RollbackDebug = CreateDefaultSubobject<URollbackDebugComponent>(TEXT("RollbackDebug"));
	RollbackDebug->SetNetAddressable(); // Make DSO components net addressable
	RollbackDebug->SetIsReplicated(true); // Enable replication by default

	ShapeManager = CreateDefaultSubobject<UShapeManagerComponent>(TEXT("ShapeManager"));
	ShapeManager->ShapeSourceMesh = GetMesh();

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.
}

void AFPSTemplateCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	/*
	if (!GetWorld()->IsServer()) {
		GetMesh()->SetOwnerNoSee(true);
	}
	*/

	CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(StartWeaponClass);
	CurrentWeapon->SetOwnerCharacter(this);
	bool ownedLocally = IsLocallyControlled();
	//Attach gun to Skeleton, doing it here because the skeleton is not yet created in the constructor
	if (ownedLocally) {
		CurrentWeapon->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	}
	else {
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	}
	ShapeManager->SavePhysicsShapeTransformsLocal(OriginalShapeTransforms);

}

//////////////////////////////////////////////////////////////////////////
// Input

void AFPSTemplateCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSTemplateCharacter::OnFire);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSTemplateCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSTemplateCharacter::MoveRight);

	PlayerInputComponent->BindAction("Scope", IE_Pressed, RollbackDebug, &URollbackDebugComponent::OnStartScoping);
	PlayerInputComponent->BindAction("Scope", IE_Released, RollbackDebug, &URollbackDebugComponent::OnStopScoping);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFPSTemplateCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFPSTemplateCharacter::LookUpAtRate);
}

void AFPSTemplateCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Do replications on properties
	DOREPLIFETIME(AFPSTemplateCharacter, RepViewRotation);
}

void AFPSTemplateCharacter::OnFire()
{
	/*
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// spawn the projectile at the muzzle
			World->SpawnActor<AFPSTemplateProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}
	*/
	
	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}

	// Hitscan
	FHitResult OutHit;
	FVector CameraLocation;
	FRotator CameraRotator;

	GetController()->GetPlayerViewPoint(CameraLocation, CameraRotator);
	const FVector ShootDir = CameraRotator.Vector();
	const FHitResult Hit = CurrentWeapon->TraceShot(CameraLocation, ShootDir);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Client fire time: %d %d"), FDateTime::UtcNow().GetSecond(), 
		FDateTime::UtcNow().GetMillisecond()));
	if (Hit.GetActor() != NULL) {
		AFPSTemplateCharacter* HitPlayer = Cast<AFPSTemplateCharacter>(Hit.GetActor());
		if (HitPlayer != NULL) {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Hit: %s"), *Hit.BoneName.ToString()));
			DrawDebugBox(GetWorld(), HitPlayer->GetActorLocation(), FVector(60.0f, 30.0f, 45.0f), HitPlayer->GetActorQuat(), FColor::Orange, false, 5.0f);
			HitPlayer->ShapeManager->DrawHitboxes(FColor::Orange, 5.0f);
			ServerFire(HitPlayer);
		}
	}
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

void AFPSTemplateCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFPSTemplateCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFPSTemplateCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFPSTemplateCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}


void AFPSTemplateCharacter::OnRep_ReplicatedMovement()
{
	// TODO: by overriding this, the movement state / animation systems don't seem to work properly
	//UE_LOG(LogTemp, Warning, TEXT("Receive at: %f"), GetWorld()->GetTimeSeconds());
	UCustomCharacterMovementComponent* MovementComponent = Cast<UCustomCharacterMovementComponent>(GetMovementComponent());
	FVector NewLocation = FRepMovement::RebaseOntoLocalOrigin(GetReplicatedMovement().Location, this);
	MovementComponent->OnReceiveServerUpdate(NewLocation, GetReplicatedMovement().Rotation.Quaternion(), GetReplicatedMovement().LinearVelocity, NetUpdateFrequency);
}

void AFPSTemplateCharacter::OnRep_RepViewRotation()
{
	RepViewRotationTimeline.AddSnapshotCompensating(RepViewRotationSnapshot(RepViewRotation), GetWorld()->GetTimeSeconds(), 
		GetInterpolationTime(), NetUpdateFrequency);
}

FRotator AFPSTemplateCharacter::GetViewRotation()
{
	if (GetController() != NULL) {
		return GetController()->GetControlRotation();
	}
	else {
		return RepViewRotationTimeline.GetSnapshot(GetInterpolationTime()).ViewRotation;
	}
}

FVector AFPSTemplateCharacter::GetPlayerVelocity()
{
	UCustomCharacterMovementComponent* MovementComponent = Cast<UCustomCharacterMovementComponent>(GetMovementComponent());
	if (GetLocalRole() != ROLE_SimulatedProxy) {
		return MovementComponent->Velocity;
	}
	else {
		return MovementComponent->GetRepVelocity();
	}
}

void AFPSTemplateCharacter::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
	if (GetLocalRole() == ROLE_Authority)
	{
		// Add a movement snapshot
		AFPSTemplateGameMode* GameMode = (AFPSTemplateGameMode *) GetWorld()->GetAuthGameMode();
		if (GameMode != NULL) 
		{
			RepTimeline<RepSnapshot>& Timeline = GameMode->GetRepMovementTimeline((IRepMovable*) this);
			// After Super::PreReplication has been called, ReplicatedMovement should be updated
			//UE_LOG(LogTemp, Warning, TEXT("Server add at: %f"), GetWorld()->GetTimeSeconds());
			Timeline.AddSnapshot(RepSnapshot(GetReplicatedMovement().Location, GetReplicatedMovement().Rotation.Quaternion(), 
				GetReplicatedMovement().LinearVelocity), GetWorld()->GetTimeSeconds());
		}

		// Update the RepViewRotation member for replication
		RepViewRotation = GetController()->GetControlRotation();
	}
}

void AFPSTemplateCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Server saves animation snapshot
	AFPSTemplateGameMode* GameMode = (AFPSTemplateGameMode *)GetWorld()->GetAuthGameMode();
	if (GameMode != NULL) {
		RepTimeline<RepAnimationSnapshot>& AnimationTimeline = GameMode->GetRepAnimationTimeline((IRepMovable*)this);
		TMap<physx::PxShape*, physx::PxTransform> ShapeTransforms;
		ShapeManager->SavePhysicsShapeTransformsGlobal(ShapeTransforms);
		AnimationTimeline.AddSnapshot(RepAnimationSnapshot(ShapeTransforms), GetWorld()->GetTimeSeconds());
	}
}

void AFPSTemplateCharacter::ServerFire_Implementation(AFPSTemplateCharacter* Target)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Fire!"));
	AFPSTemplateGameMode* GameMode = (AFPSTemplateGameMode *)GetWorld()->GetAuthGameMode();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Server fire time: %d %d"), FDateTime::UtcNow().GetSecond(), 
		FDateTime::UtcNow().GetMillisecond()));
	if (GameMode != NULL) {
		RollbackDebug->ServerSendShapeTransforms(Target, ServerReplicationMessageType::ServerState);

		FVector ServerPosition = Target->GetActorLocation();
		FQuat ServerRotation = Target->GetActorQuat();

		GameMode->GetRepWorldTimelines().PreRollbackWorld((IRepMovable*)this), 
		GameMode->GetRepWorldTimelines().RollbackWorld((IRepMovable*)this, GetWorld()->GetTimeSeconds(), 
			RepTimeline<RepSnapshot>::InterpolationOffset, GetPing());
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("Ping at server: %f"), GetPingRaw()));
		RollbackDebug->ServerSendShapeTransforms(Target, ServerReplicationMessageType::RollbackState);
		FVector RollbackPosition = Target->GetActorLocation();
		FQuat RollbackRotation = Target->GetActorQuat();
		GameMode->GetRepWorldTimelines().ResetWorld((IRepMovable*)this);
		ClientConfirmHit(Target, RollbackPosition, RollbackRotation, ServerPosition, ServerRotation);

	}
}


void AFPSTemplateCharacter::ClientConfirmHit_Implementation(AFPSTemplateCharacter* HitPlayer, FVector RollbackPosition, 
	FQuat RollbackRotation, FVector ServerPosition, FQuat ServerRotation)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Hit fire time: %d %d"), FDateTime::UtcNow().GetSecond(), 
		FDateTime::UtcNow().GetMillisecond()));
	DrawDebugBox(GetWorld(), RollbackPosition , FVector(60.0f, 30.0f, 45.0f), RollbackRotation, FColor::Yellow, false, 5.0f);
	DrawDebugBox(GetWorld(), ServerPosition , FVector(60.0f, 30.0f, 45.0f), ServerRotation, FColor::Green, false, 5.0f);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Player: %s"), *RollbackPosition.ToString()));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Player: %s"), *ServerPosition.ToString()));
}

void AFPSTemplateCharacter::PrepareRollback()
{
	PreRollbackSnapshot = RepSnapshot(GetActorLocation(), GetActorQuat(), GetVelocity());
}

void AFPSTemplateCharacter::RollbackMovement(const RepSnapshot& RollbackSnapshot)
{
	UCustomCharacterMovementComponent* MovementComponent = Cast<UCustomCharacterMovementComponent>(GetMovementComponent());
	MovementComponent->ApplySnapshot(RollbackSnapshot);
}

void AFPSTemplateCharacter::RollbackAnimation(RepAnimationSnapshot& RollbackSnapshot)
{
	ApplyAnimationSnapshot(RollbackSnapshot);
}

void AFPSTemplateCharacter::ResetRollback()
{
	UCustomCharacterMovementComponent* MovementComponent = Cast<UCustomCharacterMovementComponent>(GetMovementComponent());
	MovementComponent->ApplySnapshot(PreRollbackSnapshot);
	ShapeManager->SetPhysicsShapeTransformsLocal(OriginalShapeTransforms);
}

void AFPSTemplateCharacter::ApplyAnimationSnapshot(RepAnimationSnapshot& RollbackSnapshot)
{
	ShapeManager->SetPhysicsShapeTransformsGlobal(RollbackSnapshot.ShapeTransforms);
}

float AFPSTemplateCharacter::GetPing()
{
	// TODO: the multiplication with 4 here is only because the custom PktLag seems to add only 1/2 its RTT to this variable?
	return GetPlayerState()->Ping * 0.001f * 4.0f  + RollbackOffset;
}

float AFPSTemplateCharacter::GetPingRaw()
{
	return GetPlayerState()->Ping;
}

float AFPSTemplateCharacter::GetInterpolationTime()
{
	return GetWorld()->GetTimeSeconds() - RepTimeline<RepSnapshot>::InterpolationOffset;
}

