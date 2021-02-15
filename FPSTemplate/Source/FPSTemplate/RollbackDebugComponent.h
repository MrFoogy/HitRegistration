// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "REpAnimationSnapshot.h"
#include "RollbackLogger.h"
#include "RollbackDebugComponent.generated.h"

class AFPSTemplateCharacter;

UENUM()
enum ServerReplicationMessageType
{
	ServerState, RollbackState
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSTEMPLATE_API URollbackDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URollbackDebugComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UFUNCTION(Server, Reliable)
	void ServerRequestAnimState(AFPSTemplateCharacter* Target, int Counter);

	UFUNCTION(Server, Reliable)
	void ServerStartLocalShapeTransmission(AFPSTemplateCharacter* Target, int Counter);

	UFUNCTION(Server, Reliable)
	void ServerSendLocalShape(AFPSTemplateCharacter* Target, int Counter, int ShapeID, FVector Position, FQuat Rotation);

	UFUNCTION(Client, Reliable)
	void ClientSendRollbackShape(AFPSTemplateCharacter* Target, int Counter, int ShapeID, FVector Position, FQuat Rotation);

	UFUNCTION(Server, Reliable)
	void ServerSetInitialTransform(FVector Position, FQuat Rotation);

	UFUNCTION(Server, Reliable)
	void ServerSetReplicationOffset(float Offset);

	UFUNCTION(Client, Reliable)
	void ClientDisplayShapeTransform(AFPSTemplateCharacter* Target, int ShapeID, FVector Position, FQuat Rotation, ServerReplicationMessageType Type);

	UFUNCTION(Exec, Category = ExecFunctions)
	void StartDebugMovement();

	UFUNCTION(Exec, Category = ExecFunctions)
	void SaveRollbackLog();

	UFUNCTION(Exec, Category = ExecFunctions)
	void RollbackDebugOffset(float Offset);

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	AFPSTemplateCharacter* OwnerCharacter;

	TArray<RepAnimationSnapshot> PosesLocal;
	TArray<RepAnimationSnapshot> PosesRemote;
	TArray<float> LocalPoseTimes;
	bool DebugIsMonitoring = false;
	float LastDebugShapeSendTime = 0.0f;
	float DebugShapeDisplayTime = 0.0f;
	float DebugShapeDisplayOffset = 0.0f;
	class URollbackTimelineWidget* RollbackTimelineWidget;
	FRollbackLogger RollbackLogger;

	bool IsScoping = false;

	bool ShouldUpdateTimelineSlider = true;
	bool ShouldInterpolateDebugPoses = false;

	bool IsUsingDebugMovement = false;
	float DebugMovementStartTime;


	const int NUM_RANDOM_HIT_TESTS = 300;
	TArray<FVector> RandomBoundingBoxPositions;
	TArray<bool> RandomHitTestResults;
	void GenerateRandomBoundingBoxPositions();
	FVector GetRandomPointInBoundingBox();
	FRay GetRandomCollisionTestRay(int RandomPointIndex);
	float CalculateRandomHitRate(RepAnimationSnapshot& RollbackSnapshot);
	bool TestHitFromRay(const FRay& Ray);

	void FindOptimalRollbackFudge(int Counter);

public:
	virtual void ServerSendShapeTransforms(AFPSTemplateCharacter* Target, ServerReplicationMessageType Type);
	virtual void DisplayShapeTransform(int ShapeID, FVector Position, FQuat Rotation, ServerReplicationMessageType Type);

	int AnimSaveCounter = 0;

	void SaveLocalShapeForDebug();
	void OnStartScoping();
	void OnStopScoping();

	void OnReceiveRemoteShape(int Counter, int ShapeID, FVector Position, FQuat Rotation);

	void DebugPrepareMonitoredTest();
	void DebugPrepareMonitoringTest();
	UFUNCTION(Exec, Category = ExecFunctions)
	void DebugStartMonitoring();

	AFPSTemplateCharacter* DebugFindOtherPlayer();

	void SetRollbackTimelineValue(float Value);
	void DebugSetShapeDisplayOffset(float Offset);
	void OnOpenRollbackTimelineUI(URollbackTimelineWidget* Widget);
	void SetShouldUpdateTimelineSlider(bool ShouldUpdateSlider);
	void SetShouldInterpolateDebugPoses(bool ShouldInterpolatePoses);
};
