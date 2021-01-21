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
#include "RollbackLogger.h"
#include "DebugUtil.h"
#include "PhysXIncludes.h"
#include "PhysicsPublic.h"	
#include "Runtime/Engine/Private/PhysicsEngine/PhysXSupport.h"
#include "GauntletModule.h"
#include "FPSTemplateCharacter.generated.h"

class UInputComponent;
class AFPSTemplateCharacter;

UENUM()
enum ServerReplicationMessageType
{
	ServerState, RollbackState
};

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

	/*
	AFPSTemplateCharacter();
public:
	AFPSTemplateCharacter(const FObjectInitializer& ObjectInitializer);
	*/

protected:
	virtual void BeginPlay();
	RepSnapshot PreRollbackSnapshot;
	TMap<physx::PxShape*, physx::PxTransform> OriginalShapeTransforms;
	TArray<physx::PxShape*> AllShapes;
	TMap<physx::PxShape*, int> ShapeIDs;

	RepTimeline<RepViewRotationSnapshot> RepViewRotationTimeline;

	TArray<RepAnimationSnapshot> PosesLocal;
	TArray<RepAnimationSnapshot> PosesRollback;
	TArray<float> LocalPoseTimes;
	int AnimSaveCounter = 0;
	int ReplicationCounter = 0;
	bool DebugIsMonitoring = false;
	float LastDebugShapeSendTime = 0.0f;
	float DebugShapeDisplayTime = 0.0f;
	class URollbackTimelineWidget* RollbackTimelineWidget;
	FRollbackLogger RollbackLogger;

	bool IsScoping = false;
	bool ShouldUpdateTimelineSlider = true;
	bool ShouldInterpolateDebugPoses = false;

	float RollbackOffset = -0.035f;

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

	UFUNCTION(Exec, Category = ExecFunctions)
	void StartDebugMovement();

	UFUNCTION(Exec, Category = ExecFunctions)
	void SaveRollbackLog();

	UFUNCTION(Exec, Category = ExecFunctions)
	void RollbackDebugOffset(float Offset);

	UFUNCTION(BlueprintCallable)
	FRotator GetViewRotation();

	UFUNCTION(BlueprintCallable)
	FVector GetPlayerVelocity();

protected:
	/** Fires a projectile. */
	void OnFire();

	void OnStartScoping();

	void OnStopScoping();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

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

	void DrawHitboxes(const FColor& Color);

	void TestDisplaceHitboxes();

	bool IsUsingDebugMovement = false;
	float DebugMovementStartTime;

	void SavePhysicsShapeTransformsLocal(TMap<physx::PxShape*, physx::PxTransform>& OutTransforms);

	void SetPhysicsShapeTransformsLocal(TMap<physx::PxShape*, physx::PxTransform>& Transforms);

	void SavePhysicsShapeTransformsGlobal(TMap<physx::PxShape*, physx::PxTransform>& OutTransforms);

	void SetPhysicsShapeTransformsGlobal(TMap<physx::PxShape*, physx::PxTransform>& Transforms);

	void OnReceiveRollbackShape(int Counter, int ShapeID, FVector Position, FQuat Rotation);

	void SaveLocalShapeForDebug();

	/** 
	 * Calls a function for each Physics Shape on the character
	 * @param Function the function to be called
	 */
	template<typename F>
	void PerformPhysicsShapeOperation(F Function);

	virtual void ApplyAnimationSnapshot(RepAnimationSnapshot& AnimationSnapshot);

	virtual void ServerSendShapeTransforms(AFPSTemplateCharacter* Target, ServerReplicationMessageType Type);
	virtual void DisplayShapeTransform(int ShapeID, FVector Position, FQuat Rotation, ServerReplicationMessageType Type);

	virtual float GetPing();

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

	UFUNCTION(Server, Reliable)
	void ServerRequestAnimState(AFPSTemplateCharacter* Target, int Counter);

	UFUNCTION(Server, Reliable)
	void ServerSetReplicationOffset(float Offset);

	UFUNCTION(Server, Reliable)
	void ServerSetInitialTransform(FVector Position, FQuat Rotation);

	UFUNCTION(Client, Reliable)
	void ClientConfirmHit(AFPSTemplateCharacter* HitPlayer, FVector RollbackPosition, FQuat RollbackRotation, FVector ServerPosition, FQuat ServerRotation);

	UFUNCTION(Client, Reliable)
	void ClientDisplayShapeTransform(AFPSTemplateCharacter* Target, int ShapeID, FVector Position, FQuat Rotation, ServerReplicationMessageType Type);

	UFUNCTION(Client, Reliable)
	void ClientSendRollbackShape(AFPSTemplateCharacter* Target, int Counter, int ShapeID, FVector Position, FQuat Rotation);

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
	virtual float GetInterpolationTime();

	void DebugPrepareMonitoredTest();
	void DebugPrepareMonitoringTest();
	void DebugStartMonitoring();
	void SetRollbackTimelineValue(float Value);
	AFPSTemplateCharacter* DebugFindOtherPlayer();
	void OnOpenRollbackTimelineUI(URollbackTimelineWidget* Widget);
	void SetShouldUpdateTimelineSlider(bool ShouldUpdateSlider);
	void SetShouldInterpolateDebugPoses(bool ShouldInterpolatePoses);
};
