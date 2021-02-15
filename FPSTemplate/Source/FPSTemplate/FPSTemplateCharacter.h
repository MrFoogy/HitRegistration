// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameState.h"
#include "Weapon.h"
#include "Runtime/Engine/Public/EngineGlobals.h"
#include "RepMovable.h"
#include "FPSTemplateGameMode.h"
#include "RepTimeline.h"
#include "RepViewRotationSnapshot.h"
#include "RepSnapshot.h"
#include "RepAnimationSnapshot.h"
#include "RollbackTimelineWidget.h"
#include "DebugUtil.h"
#include "PhysXIncludes.h"
#include "PhysicsPublic.h"	
#include "Runtime/Engine/Private/PhysicsEngine/PhysXSupport.h"
#include "GauntletModule.h"
#include "FPSTemplateCharacter.generated.h"

class UInputComponent;
class AFPSTemplateCharacter;
class URollbackDebugComponent;
class UShapeManagerComponent;

UCLASS(config=Game)
class AFPSTemplateCharacter : public ACharacter, public IRepMovable
{
	GENERATED_UCLASS_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

public:
	UPROPERTY(VisibleAnywhere, Category = "HitReg")
	class URollbackDebugComponent* RollbackDebug;

	UPROPERTY(VisibleAnywhere, Category = "HitReg")
	class UShapeManagerComponent* ShapeManager;

	/*
	AFPSTemplateCharacter();
public:
	AFPSTemplateCharacter(const FObjectInitializer& ObjectInitializer);
	*/

public:
	float RollbackOffset = -0.035f;

protected:
	virtual void BeginPlay();
	RepSnapshot PreRollbackSnapshot;
	TMap<physx::PxShape*, physx::PxTransform> OriginalShapeTransforms;

	RepTimeline<RepViewRotationSnapshot> RepViewRotationTimeline;


public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AFPSTemplateProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** Weapon class to start with */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AWeapon> StartWeaponClass;

	UPROPERTY(Transient)
	class AWeapon* CurrentWeapon;

	UFUNCTION(BlueprintCallable)
	FRotator GetViewRotation();

	UFUNCTION(BlueprintCallable)
	FVector GetPlayerVelocity();

public:
	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	virtual void ApplyAnimationSnapshot(RepAnimationSnapshot& AnimationSnapshot);

protected:
	/** Fires a projectile. */
	void OnFire();

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

public:
	UFUNCTION(Server, Reliable)
	void ServerFire(AFPSTemplateCharacter* Target);


	UFUNCTION(Client, Reliable)
	void ClientConfirmHit(AFPSTemplateCharacter* HitPlayer, FVector RollbackPosition, FQuat RollbackRotation, FVector ServerPosition, FQuat ServerRotation);

	UPROPERTY(Transient, Replicated, ReplicatedUsing=OnRep_RepViewRotation)
	FRotator RepViewRotation;

	UFUNCTION()
	virtual void OnRep_RepViewRotation();

public:
	virtual void OnRep_ReplicatedMovement() override;
	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;
	virtual void Tick( float DeltaSeconds ) override;
	virtual void PrepareRollback() override;
	virtual void RollbackMovement(const RepSnapshot& RepSnapshot) override;
	virtual void RollbackAnimation(RepAnimationSnapshot& RepSnapshot) override;
	virtual void ResetRollback() override;

public:
	virtual float GetPing();
	virtual float GetPingRaw();
	virtual float GetInterpolationTime();
};
