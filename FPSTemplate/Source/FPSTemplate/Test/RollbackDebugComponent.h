// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/Timelines/RepAnimationSnapshot.h"
#include "Test/RollbackLogger.h"
#include "Core/ReplicationUtil.h"
#include "RollbackDebugComponent.generated.h"

class AFPSTemplateCharacter;

UENUM()
enum ServerReplicationMessageType
{
	ServerState, RollbackState
};

UENUM()
enum AutomatedMovementType
{
	MoveStraight, Alternate, None
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
	void ServerRequestAnimState(AFPSTemplateCharacter* Target, int Counter, MovementReplicationType ReplicationType);

	UFUNCTION(Server, Reliable)
	void ServerStartLocalShapeTransmission(AFPSTemplateCharacter* Target, int Counter, FVector Position, FQuat Rotation, MovementReplicationType ReplicationType);

	UFUNCTION(Server, Reliable)
	void ServerSendLocalShape(AFPSTemplateCharacter* Target, int Counter, int ShapeID, FVector Position, FQuat Rotation);

	UFUNCTION(Client, Reliable)
	void ClientSendRollbackShape(AFPSTemplateCharacter* Target, int Counter, int ShapeID, FVector Position, FQuat Rotation);

	UFUNCTION(Server, Reliable)
	void ServerSetInitialTransform(FVector Position, FQuat Rotation);

	UFUNCTION(Server, Reliable)
	void ServerSetReplicationOffset(float Offset);

	UFUNCTION(Server, Reliable)
	void ServerSetLatency(int Latency);

	UFUNCTION(Client, Reliable)
	void ClientDisplayShapeTransform(AFPSTemplateCharacter* Target, int ShapeID, FVector Position, FQuat Rotation, ServerReplicationMessageType Type, float duration);

	UFUNCTION(Client, Reliable)
	void ClientSendDebugOptimalFudge(float Time, float OptimalFudge, float OptimalAngDiff, float OptimalPosDiff, float OptimalHitRate, float TransmissionTime);

	void DebugStartMonitoring();

	void StartDebugMovement(AutomatedMovementType MovementType);

	void SaveRollbackLog();

	UFUNCTION(Exec, Category = ExecFunctions)
	void RollbackDebugOffset(float Offset);

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	AFPSTemplateCharacter* OwnerCharacter;

	TArray<FVector> PositionsClient;
	TArray<FQuat> RotationsClient;
	TArray<RepAnimationSnapshot> PosesLocal;
	TArray<RepAnimationSnapshot> PosesRemote;
	TArray<float> LocalPoseTimes;
	TArray<FDateTime> ClientSendTimes;
	bool DebugIsMonitoring = false;
	float LastDebugShapeSendTime = 0.0f;
	float DebugShapeDisplayTime = 0.0f;
	float DebugShapeDisplayOffset = 0.0f;
	class URollbackTimelineWidget* RollbackTimelineWidget;
	FRollbackLogger RollbackLogger;

	int DrawRayCounter = 5;

	bool IsScoping = false;

	bool ShouldUpdateTimelineSlider = true;
	bool ShouldInterpolateDebugPoses = false;

	AutomatedMovementType DebugMovementType = AutomatedMovementType::None;
	float DebugMovementStartTime;


	const int NUM_RANDOM_HIT_TESTS = 300;
	TArray<FVector> RandomBoundingBoxPositions;
	TArray<bool> RandomHitTestResults;
	void GenerateRandomBoundingBoxPositions(FVector& CenterPosition, FQuat& CenterRotation);
	FVector GetRandomPointInBoundingBox(FVector& CenterPosition, FQuat& CenterRotation);
	FRay GetRandomCollisionTestRay(int RandomPointIndex);
	float CalculateRandomHitRate(FVector& Posisition, FQuat& Rotation, RepAnimationSnapshot& LocalSnapshot, RepAnimationSnapshot& RollbackSnapshot);
	bool TestHitFromRay(const FRay& Ray);

	void FindOptimalRollbackFudge(int Counter, float& OptimalFudge, float& OptimalAngDiff, float& OptimalPosDiff, MovementReplicationType ReplicationType);

public:
	bool IsMonitoringDiscrepancy = false;

	virtual void ServerSendShapeTransforms(AFPSTemplateCharacter* Target, ServerReplicationMessageType Type);
	virtual void DisplayShapeTransform(int ShapeID, FVector Position, FQuat Rotation, ServerReplicationMessageType Type, float Duration);

	void SaveLocalPose(int Counter);

	int AnimSaveCounter = 0;

	void SaveLocalShapeForDebug();
	void OnStartScoping();
	void OnStopScoping();

	void OnServerReceiveRemoteShape(AFPSTemplateCharacter* MonitoringPlayer, int Counter, int ShapeID, FVector Position, FQuat Rotation);
	void OnClientReceiveRemoteShape(AFPSTemplateCharacter* MonitoringPlayer, int Counter, int ShapeID, FVector Position, FQuat Rotation);

	void DebugPrepareMonitoredTest();
	void DebugPrepareMonitoringTest(MovementReplicationType ReplicationType, FString LogFileName);

	AFPSTemplateCharacter* DebugFindOtherPlayer();

	void SetRollbackTimelineValue(float Value);
	void DebugSetShapeDisplayOffset(float Offset);
	void OnOpenRollbackTimelineUI(URollbackTimelineWidget* Widget);
	void SetShouldUpdateTimelineSlider(bool ShouldUpdateSlider);
	void SetShouldInterpolateDebugPoses(bool ShouldInterpolatePoses);
};
