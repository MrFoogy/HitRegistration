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
#include "Misc/CString.h"

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
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
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
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	IsUsingDebugMovement = false;

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.
}

void AFPSTemplateCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

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
	SavePhysicsShapeTransformsLocal(OriginalShapeTransforms);

	auto ShapeFunction = [&ShapesArray = AllShapes, &IDMap = ShapeIDs](physx::PxRigidActor* PxActor, physx::PxShape* PxShape, int ID)
	{
		physx::PxTransform ActorGlobalPose = PxActor->getGlobalPose();
		physx::PxTransform ShapeGlobalPose = ActorGlobalPose * PxShape->getLocalPose();
		if (ShapesArray.Num() < ID + 1) {
			ShapesArray.SetNum(ID + 1);
		}
		ShapesArray[ID] = PxShape;
		IDMap.Add(PxShape, ID);
	};
	PerformPhysicsShapeOperation(ShapeFunction);
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
	if (Hit.GetActor() != NULL) {
		AFPSTemplateCharacter* HitPlayer = Cast<AFPSTemplateCharacter>(Hit.GetActor());
		if (HitPlayer != NULL) {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Hit: %s"), *Hit.BoneName.ToString()));
			DrawDebugBox(GetWorld(), HitPlayer->GetActorLocation(), FVector(60.0f, 30.0f, 45.0f), HitPlayer->GetActorQuat(), FColor::Orange, 1.0f);
			HitPlayer->DrawHitboxes(FColor::Orange);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Ping at client: %f"), GetPing()));
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
	UCustomCharacterMovementComponent* MovementComponent = Cast<UCustomCharacterMovementComponent>(GetMovementComponent());
	FVector NewLocation = FRepMovement::RebaseOntoLocalOrigin(ReplicatedMovement.Location, this);
	MovementComponent->OnReceiveServerUpdate(NewLocation, ReplicatedMovement.Rotation.Quaternion(), ReplicatedMovement.LinearVelocity);
}

void AFPSTemplateCharacter::OnRep_RepViewRotation()
{
	RepViewRotationTimeline.AddSnapshot(RepViewRotationSnapshot(RepViewRotation), GetWorld()->GetTimeSeconds());
}

FRotator AFPSTemplateCharacter::GetViewRotation()
{
	if (GetController() != NULL) {
		return GetController()->GetControlRotation();
	}
	else {
		return RepViewRotationTimeline.GetSnapshot(GetWorld()->GetTimeSeconds() - RepTimeline<RepSnapshot>::InterpolationOffset).ViewRotation;
	}
}

FVector AFPSTemplateCharacter::GetPlayerVelocity()
{
	UCustomCharacterMovementComponent* MovementComponent = Cast<UCustomCharacterMovementComponent>(GetMovementComponent());
		return MovementComponent->GetRepVelocity();
	if (GetController() != NULL) {
		return MovementComponent->Velocity;
	}
	else {
	}
}

void AFPSTemplateCharacter::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
	if (Role == ROLE_Authority)
	{
		// Add a movement snapshot
		AFPSTemplateGameMode* GameMode = (AFPSTemplateGameMode *) GetWorld()->GetAuthGameMode();
		if (GameMode != NULL) 
		{
			RepTimeline<RepSnapshot>& Timeline = GameMode->GetRepMovementTimeline((IRepMovable*) this);
			// After Super::PreReplication has been called, ReplicatedMovement should be updated
			UE_LOG(LogTemp, Warning, TEXT("Add snapshot time: %f"), GetWorld()->GetTimeSeconds());
			Timeline.AddSnapshot(RepSnapshot(ReplicatedMovement.Location, ReplicatedMovement.Rotation.Quaternion(), 
				ReplicatedMovement.LinearVelocity), GetWorld()->GetTimeSeconds());
		}

		// Update the RepViewRotation member for replication
		RepViewRotation = GetController()->GetControlRotation();
	}
}

void AFPSTemplateCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (IsUsingDebugMovement)
	{
		MoveRight(FMath::Sin(GetWorld()->GetTimeSeconds()));
	}
	AFPSTemplateGameMode* GameMode = (AFPSTemplateGameMode *)GetWorld()->GetAuthGameMode();
	if (GameMode != NULL) {
		RepTimeline<RepAnimationSnapshot>& AnimationTimeline = GameMode->GetRepAnimationTimeline((IRepMovable*)this);
		TMap<physx::PxShape*, physx::PxTransform> ShapeTransforms;
		SavePhysicsShapeTransformsGlobal(ShapeTransforms);
		AnimationTimeline.AddSnapshot(RepAnimationSnapshot(ShapeTransforms), GetWorld()->GetTimeSeconds());
	}
}

void AFPSTemplateCharacter::StartDebugMovement()
{
	if (HasLocalNetOwner()) {
		IsUsingDebugMovement = true;
	}
}

void AFPSTemplateCharacter::ServerFire_Implementation(AFPSTemplateCharacter* Target)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Fire!"));
	AFPSTemplateGameMode* GameMode = (AFPSTemplateGameMode *)GetWorld()->GetAuthGameMode();
	if (GameMode != NULL) {
		ServerSendShapeTransforms(Target, ServerReplicationMessageType::ServerState);

		FVector ServerPosition = Target->GetActorLocation();
		FQuat ServerRotation = Target->GetActorQuat();

		GameMode->GetRepWorldTimelines().PreRollbackWorld((IRepMovable*)this), 
		// TODO: the multiplication with 2 here is only because the custom PktLag seems to add only 1/2 its RTT to this variable?
		GameMode->GetRepWorldTimelines().RollbackWorld((IRepMovable*)this, GetWorld()->GetTimeSeconds(), 
			RepMovementTimeline::InterpolationOffset, GetPing() * 4.0f * 0.001f);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("Ping at server: %f"), GetPing()));
		ServerSendShapeTransforms(Target, ServerReplicationMessageType::RollbackState);
		FVector RollbackPosition = Target->GetActorLocation();
		FQuat RollbackRotation = Target->GetActorQuat();
		GameMode->GetRepWorldTimelines().ResetWorld((IRepMovable*)this);
		ClientConfirmHit(Target, RollbackPosition, RollbackRotation, ServerPosition, ServerRotation);

	}
	/*
	DrawHitboxes(FColor::Purple);
	FVector SaveLocation = GetActorLocation();

	SetActorLocation(FVector(0.0f, 0.0f, 150.0f), false, (FHitResult* ) nullptr, ETeleportType::TeleportPhysics);
	TestDisplaceHitboxes();

	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams;
	TraceParams.bReturnPhysicalMaterial = true;
	float WeaponRange = 100000.0f;
	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingleByChannel(Hit, FVector(0.0f, 200.0f, 200.0f), FVector(0.0f, -200.0f, 200.0f), ECC_Visibility, TraceParams);
	DrawDebugLine(GetWorld(), FVector(0.0f, 200.0f, 200.0f), FVector(0.0f, -200.0f, 200.0f), FColor::Purple, false, 2.0f, 0, 1.f);
	if (Hit.GetActor() != NULL) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, (TEXT("Hit: ???")));
		AFPSTemplateCharacter* HitPlayer = Cast<AFPSTemplateCharacter>(Hit.GetActor());
		if (HitPlayer != NULL) {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("Hit: %s"), *Hit.BoneName.ToString()));
		}
	}
	DrawHitboxes(FColor::Cyan);

	SetPhysicsShapeTransformsLocal(OriginalShapeTransforms);
	SetActorLocation(SaveLocation, false, (FHitResult* ) nullptr, ETeleportType::TeleportPhysics);
	DrawHitboxes(FColor::Green);
	*/
}

void AFPSTemplateCharacter::ClientConfirmHit_Implementation(AFPSTemplateCharacter* HitPlayer, FVector RollbackPosition, 
	FQuat RollbackRotation, FVector ServerPosition, FQuat ServerRotation)
{
	DrawDebugBox(GetWorld(), RollbackPosition , FVector(60.0f, 30.0f, 45.0f), FQuat::Identity, FColor::Yellow, 1.0f);
	DrawDebugBox(GetWorld(), ServerPosition , FVector(60.0f, 30.0f, 45.0f), FQuat::Identity, FColor::Green, 1.0f);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Player: %s"), *RollbackPosition.ToString()));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Player: %s"), *ServerPosition.ToString()));
}

void AFPSTemplateCharacter::ClientDisplayShapeTransform_Implementation(AFPSTemplateCharacter* Target, int ShapeID, 
	FVector Position, FQuat Rotation, ServerReplicationMessageType Type)
{
	Target->DisplayShapeTransform(ShapeID, Position, Rotation, Type);
}

void AFPSTemplateCharacter::DisplayShapeTransform(int ShapeID, 
	FVector Position, FQuat Rotation, ServerReplicationMessageType Type)
{
	physx::PxShape* Shape = AllShapes[ShapeID];
	DebugUtil::DrawPxShape(GetWorld(), Shape, Position, Rotation, Type == ServerReplicationMessageType::ServerState ? FColor::Green : FColor::Yellow);
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
	SetPhysicsShapeTransformsLocal(OriginalShapeTransforms);
}

void AFPSTemplateCharacter::ApplyAnimationSnapshot(RepAnimationSnapshot& RollbackSnapshot)
{
	SetPhysicsShapeTransformsGlobal(RollbackSnapshot.ShapeTransforms);
}

void AFPSTemplateCharacter::ServerSendShapeTransforms(AFPSTemplateCharacter* Target, ServerReplicationMessageType Type)
{
	for (int i = 0; i < Target->AllShapes.Num(); i++) {
		PxRigidActor* PxActor = (PxRigidActor*) Target->AllShapes[i]->getActor();
		PxTransform ShapeGlobalPose = PxActor->getGlobalPose() * Target->AllShapes[i]->getLocalPose();
		ClientDisplayShapeTransform(Target, i, P2UVector(ShapeGlobalPose.p), P2UQuat(ShapeGlobalPose.q), Type);
	}
}

template<typename F>
void AFPSTemplateCharacter::PerformPhysicsShapeOperation(F Function)
{
	FBodyInstance* PhysicsBody = GetMesh()->GetBodyInstance();
	PxScene* Scene = GetWorld()->GetPhysicsScene()->GetPxScene();
	TArray<FPhysicsShapeHandle> collisionShapes;
	Scene->lockRead();
	int32 numSyncShapes = PhysicsBody->GetAllShapes_AssumesLocked(collisionShapes);
	Scene->unlockRead();
	int NumActors = collisionShapes[0].Shape->getActor()->getAggregate()->getNbActors();

	physx::PxActor** PxActors = new physx::PxActor*[NumActors];
	physx::PxShape* PxShapes[10];

	int FoundActors = collisionShapes[0].Shape->getActor()->getAggregate()->getActors(PxActors, NumActors);
	int Index = 0;
	for (int i = 0; i < FoundActors; i++) {
		physx::PxRigidActor* RigidActor = (physx::PxRigidActor*) PxActors[i];
		if (RigidActor == NULL) continue;
		int FoundShapes = RigidActor->getShapes(PxShapes, 10);
		for (int j = 0; j < FoundShapes; j++) {
			Function(RigidActor, PxShapes[j], Index);
			Index++;
		}
	}
	delete[] PxActors;
}

void AFPSTemplateCharacter::DrawHitboxes(const FColor& Color) 
{
	auto World = GetWorld();
	auto ShapeFunction = [Color, World](physx::PxRigidActor* PxActor, physx::PxShape* PxShape, int ID)
	{
		DebugUtil::DrawPxShape(World, PxActor, PxShape, Color);
	};
	PerformPhysicsShapeOperation(ShapeFunction);
}

void AFPSTemplateCharacter::TestDisplaceHitboxes()
{
	auto ShapeFunction = [](physx::PxRigidActor* PxActor, physx::PxShape* PxShape, int ID)
	{
		physx::PxTransform DesiredGlobalTransform(physx::PxVec3(0.0f, 0.0f, 200.0f), physx::PxQuat(physx::PxIdentity));
		physx::PxTransform ActorGlobalPose = PxActor->getGlobalPose();
		PxShape->setLocalPose(physx::PxTransform(ActorGlobalPose.getInverse() * DesiredGlobalTransform));
	};
	PerformPhysicsShapeOperation(ShapeFunction);
}

void AFPSTemplateCharacter::SavePhysicsShapeTransformsLocal(TMap<physx::PxShape*, physx::PxTransform>& OutTransforms)
{
	auto ShapeFunction = [&OutTransforms](physx::PxRigidActor* PxActor, physx::PxShape* PxShape, int ID)
	{
		OutTransforms.Add(PxShape, PxShape->getLocalPose());
	};
	PerformPhysicsShapeOperation(ShapeFunction);
}

void AFPSTemplateCharacter::SetPhysicsShapeTransformsLocal(TMap<physx::PxShape*, physx::PxTransform>& Transforms)
{
	auto ShapeFunction = [&Transforms](physx::PxRigidActor* PxActor, physx::PxShape* PxShape, int ID)
	{
		PxShape->setLocalPose(Transforms[PxShape]);
	};
	PerformPhysicsShapeOperation(ShapeFunction);
}

void AFPSTemplateCharacter::SavePhysicsShapeTransformsGlobal(TMap<physx::PxShape*, physx::PxTransform>& OutTransforms)
{
	auto ShapeFunction = [&OutTransforms](physx::PxRigidActor* PxActor, physx::PxShape* PxShape, int ID)
	{
		physx::PxTransform ActorGlobalPose = PxActor->getGlobalPose();
		physx::PxTransform ShapeGlobalPose = ActorGlobalPose * PxShape->getLocalPose();
		OutTransforms.Add(PxShape, ShapeGlobalPose);
	};
	PerformPhysicsShapeOperation(ShapeFunction);
}

void AFPSTemplateCharacter::SetPhysicsShapeTransformsGlobal(TMap<physx::PxShape*, physx::PxTransform>& Transforms)
{
	UE_LOG(LogTemp, Warning, TEXT("Rollback anim"));
	auto ShapeFunction = [&Transforms](physx::PxRigidActor* PxActor, physx::PxShape* PxShape, int ID)
	{
		physx::PxTransform ActorGlobalPose = PxActor->getGlobalPose();
		PxShape->setLocalPose(ActorGlobalPose.getInverse() * Transforms[PxShape]);
	};
	PerformPhysicsShapeOperation(ShapeFunction);
}

float AFPSTemplateCharacter::GetPing()
{
	return GetPlayerState()->Ping;
}
