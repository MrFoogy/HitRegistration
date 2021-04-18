// Fill out your copyright notice in the Description page of Project Settings.


#include "Test/RollbackDebugComponent.h"
#include "Runtime/Engine/Public/EngineGlobals.h"
#include "Kismet/KismetMathLibrary.h"
#include "Core/DebugUtil.h"
#include "Core/FPSTemplateCharacter.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Core/ShapeManagerComponent.h"

// Sets default values for this component's properties
URollbackDebugComponent::URollbackDebugComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void URollbackDebugComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AFPSTemplateCharacter>(GetOwner());
}

void URollbackDebugComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

// Called every frame
void URollbackDebugComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DebugMovementType == AutomatedMovementType::Alternate) {
		float SinVal = FMath::Sin(GetWorld()->GetTimeSeconds());
		OwnerCharacter->MoveForward(SinVal > 0.0f ? 1.0f : -1.0f);
		OwnerCharacter->MoveRight(-0.5f);
	}
	if (DebugMovementType == AutomatedMovementType::MoveStraight) {
		OwnerCharacter->MoveForward(1.f);
		OwnerCharacter->MoveRight(-0.5f);
	}

	if (GetOwner()->GetLocalRole() == ROLE_SimulatedProxy) {
		if (RollbackTimelineWidget != NULL && ShouldUpdateTimelineSlider) {
			float MaxTimeDiff = LocalPoseTimes[PosesRemote.Num() - 1] - LocalPoseTimes[0];
			RollbackTimelineWidget->SetSliderMaxValue(MaxTimeDiff);
			ShouldUpdateTimelineSlider = false;
		}

		int EndIndex = FMath::Min(PosesLocal.Num(), PosesRemote.Num()) - 1;
		if (EndIndex == -1) return;

		// Local pose
		RepAnimationSnapshot LocalPose;
		int TargetIndex = EndIndex;
		if (DebugShapeDisplayTime > 0.1f) {
			for (int i = 0; i <= EndIndex; i++) {
				if (LocalPoseTimes[i] > DebugShapeDisplayTime + DebugShapeDisplayOffset) {
					TargetIndex = i;
					break;
				}
			}
		}
		int StartIndex = FMath::Max(0, TargetIndex - 1);
		if (ShouldInterpolateDebugPoses) {
			float InterpolationAlpha = UKismetMathLibrary::NormalizeToRange(DebugShapeDisplayTime + DebugShapeDisplayOffset, LocalPoseTimes[StartIndex], LocalPoseTimes[TargetIndex]);
			LocalPose = RepAnimationSnapshot::Interpolate(PosesLocal[StartIndex], PosesLocal[TargetIndex], InterpolationAlpha);
		}
		else {
			LocalPose = PosesLocal[StartIndex];
		}

		// Rollback pose
		RepAnimationSnapshot RollbackPose;
		TargetIndex = EndIndex;
		if (DebugShapeDisplayTime > 0.1f) {
			for (int i = 0; i <= EndIndex; i++) {
				if (LocalPoseTimes[i] > DebugShapeDisplayTime) {
					TargetIndex = i;
					break;
				}
			}
		}
		StartIndex = FMath::Max(0, TargetIndex - 1);
		if (ShouldInterpolateDebugPoses) {
			float InterpolationAlpha = UKismetMathLibrary::NormalizeToRange(DebugShapeDisplayTime, LocalPoseTimes[StartIndex], LocalPoseTimes[TargetIndex]);
			RollbackPose = RepAnimationSnapshot::Interpolate(PosesRemote[StartIndex], PosesRemote[TargetIndex], InterpolationAlpha);
		}
		else {
			RollbackPose = PosesRemote[StartIndex];
		}

		for (PxShape* Shape : OwnerCharacter->ShapeManager->GetAllShapes()) {
			const PxTransform& LocalTransform = LocalPose.GetShapeTransforms()[Shape];
			DebugUtil::DrawPxShape(GetWorld(), Shape, P2UVector(LocalTransform.p), P2UQuat(LocalTransform.q), FColor::Yellow, 0.0f);

			const PxTransform& RollbackTransform = RollbackPose.GetShapeTransforms()[Shape];
			DebugUtil::DrawPxShape(GetWorld(), Shape, P2UVector(RollbackTransform.p), P2UQuat(RollbackTransform.q), FColor::Orange, 0.0f);
		}
	}

	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy) {
		// Find the other players
		if (IsScoping) {
			AFPSTemplateCharacter* TargetCharacter = DebugFindOtherPlayer();
			FRotator Rot = FRotationMatrix::MakeFromX(TargetCharacter->GetActorLocation() - FVector(0.0f, 0.0f, 20.0f) - OwnerCharacter->GetActorLocation()).Rotator();
			OwnerCharacter->GetController()->SetControlRotation(Rot);
		}
		//GetWorld()->GetGameState()->PlayerArray[0]->GetPawn<AFPSTemplateCharacter>();
		if (GetWorld()->GetTimeSeconds() - LastDebugShapeSendTime > 0.2f && DebugIsMonitoring) {
			AFPSTemplateCharacter* OtherPlayer = DebugFindOtherPlayer();
			UCustomCharacterMovementComponent* OtherMovementComponent = Cast<UCustomCharacterMovementComponent>(OtherPlayer->GetCharacterMovement());
			if (IsMonitoringDiscrepancy) {
				// Have the server send rollback shape
				if (OtherPlayer != NULL) {
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Ping at client: %f"), GetPingRaw()));
					ServerRequestAnimState(OtherPlayer, OtherPlayer->RollbackDebug->AnimSaveCounter, OtherMovementComponent->ReplicationType);
					OtherPlayer->RollbackDebug->SaveLocalShapeForDebug();
				}
			}
			else {
				if (OtherPlayer != NULL) {
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Ping at client: %f"), GetPingRaw()));

					ServerStartLocalShapeTransmission(OtherPlayer, OtherPlayer->RollbackDebug->AnimSaveCounter, FDateTime::Now(), 
						OtherMovementComponent->ReplicationType);
					for (int i = 0; i < OtherPlayer->ShapeManager->GetAllShapes().Num(); i++) {
						PxRigidActor* PxActor = (PxRigidActor*)OtherPlayer->ShapeManager->GetAllShapes()[i]->getActor();
						PxTransform ShapeGlobalPose = PxActor->getGlobalPose() * OtherPlayer->ShapeManager->GetAllShapes()[i]->getLocalPose();
						ServerSendLocalShape(OtherPlayer, OtherPlayer->RollbackDebug->AnimSaveCounter, i, P2UVector(ShapeGlobalPose.p), P2UQuat(ShapeGlobalPose.q));
					}
					OtherPlayer->RollbackDebug->AnimSaveCounter++;
				}
			}

			// Send own shape to the server
			LastDebugShapeSendTime = GetWorld()->GetTimeSeconds();
		}
	}
}

void URollbackDebugComponent::SaveLocalShapeForDebug()
{
	TMap<physx::PxShape*, physx::PxTransform> ShapeTransforms;
	OwnerCharacter->ShapeManager->SavePhysicsShapeTransformsGlobal(ShapeTransforms);
	PosesLocal.Add(RepAnimationSnapshot(ShapeTransforms));
	PositionsLocal.Add(OwnerCharacter->GetActorLocation());
	RotationsLocal.Add(OwnerCharacter->GetActorQuat());
	LocalPoseTimes.Add(GetWorld()->GetTimeSeconds());
	AnimSaveCounter++;
}

void URollbackDebugComponent::OnStartScoping()
{
	IsScoping = true;
}

void URollbackDebugComponent::OnStopScoping()
{
	IsScoping = false;
}

AFPSTemplateCharacter* URollbackDebugComponent::DebugFindOtherPlayer()
{
	for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray) {
		AFPSTemplateCharacter* Character = PS->GetPawn<AFPSTemplateCharacter>();
		if (Character != OwnerCharacter) {
			return Character;
		}
	}
	return NULL;
}

//-----------------------------------------
// Debug Movement
//-----------------------------------------

void URollbackDebugComponent::StartDebugMovement(AutomatedMovementType MovementType)
{
	if (OwnerCharacter->HasLocalNetOwner()) {
		DebugMovementStartTime = GetWorld()->GetTimeSeconds();
		DebugMovementType = MovementType;
	}
}

//-----------------------------------------
// Monitoring / Gauntlet Interface
//-----------------------------------------

void URollbackDebugComponent::DebugPrepareMonitoredTest()
{
	// Player starts are randomly assigned, so for consistency, set the start position
	ServerSetInitialTransform(FVector(-300.0f, 100.0f, 262.0f), FVector(1.0f, 0.0f, 0.0f).ToOrientationQuat());
	
}

void URollbackDebugComponent::DebugPrepareMonitoringTest(MovementReplicationType ReplicationType, FString LogFileName)
{
	// Player starts are randomly assigned, so for consistency, set the start position
	ServerSetInitialTransform(FVector(0.0f, -300.0f, 262.0f), FRotationMatrix::MakeFromX(FVector(1.0f, 0.0f, 0.0f)).ToQuat());
	UCustomCharacterMovementComponent* OtherMovementComponent = Cast<UCustomCharacterMovementComponent>(DebugFindOtherPlayer()->GetCharacterMovement());
	OtherMovementComponent->ReplicationType = ReplicationType;

	RollbackLogger.CreateLogFile(LogFileName);
}

void URollbackDebugComponent::DebugStartMonitoring()
{
	DebugIsMonitoring = true;
	LastDebugShapeSendTime = GetWorld()->GetTimeSeconds();
	IsScoping = true;
}

//-----------------------------------------
// RPCS
//-----------------------------------------

void URollbackDebugComponent::ServerSetInitialTransform_Implementation(FVector Position, FQuat Rotation)
{
	OwnerCharacter->TeleportTo(Position, Rotation.Rotator());
}

void URollbackDebugComponent::ServerRequestAnimState_Implementation(AFPSTemplateCharacter* Target, int Counter, 
	MovementReplicationType ReplicationType)
{
	AFPSTemplateGameMode* GameMode = (AFPSTemplateGameMode*)GetWorld()->GetAuthGameMode();
	if (GameMode != NULL) {
		if (GameMode->ShouldUseRollback) {
			UE_LOG(LogGauntlet, Display, TEXT("Roll back!"));
			GameMode->GetRepWorldTimelines().PreRollbackTarget((IRepMovable*)Target),
				GameMode->GetRepWorldTimelines().RollbackTarget((IRepMovable*)Target, GetWorld()->GetTimeSeconds() + 0.065,
					RepTimeline<RepSnapshot>::InterpolationOffset, Target->GetPing(), ReplicationType);
		}
		for (int i = 0; i < Target->ShapeManager->GetAllShapes().Num(); i++) {
			PxRigidActor* PxActor = (PxRigidActor*)Target->ShapeManager->GetAllShapes()[i]->getActor();
			PxTransform ShapeGlobalPose = PxActor->getGlobalPose() * Target->ShapeManager->GetAllShapes()[i]->getLocalPose();
			ClientSendRollbackShape(Target, Counter, i, P2UVector(ShapeGlobalPose.p), P2UQuat(ShapeGlobalPose.q));
		}
		if (GameMode->ShouldUseRollback) {
			GameMode->GetRepWorldTimelines().ResetTarget((IRepMovable*)Target);
		}
	}
}

void URollbackDebugComponent::ServerStartLocalShapeTransmission_Implementation(AFPSTemplateCharacter* Target, int Counter, FDateTime ClientTime, 
	MovementReplicationType ReplicationType)
{
	if (Target->RollbackDebug->ClientSendTimes.Num() < Counter + 1) {
		Target->RollbackDebug->ClientSendTimes.SetNum(Counter + 1);
	}
	Target->RollbackDebug->ClientSendTimes[Counter] = ClientTime;
	Target->RollbackDebug->SaveLocalPose(Counter);
	UCustomCharacterMovementComponent* TargetMovementComponent = Cast<UCustomCharacterMovementComponent>(Target->GetCharacterMovement());
	TargetMovementComponent->ReplicationType = ReplicationType;
}

void URollbackDebugComponent::SaveLocalPose(int Counter)
{
	while (LocalPoseTimes.Num() < Counter + 1) {
		LocalPoseTimes.Add(0.0f);
	}
	LocalPoseTimes[Counter] = GetWorld()->GetTimeSeconds();
}

void URollbackDebugComponent::ServerSendLocalShape_Implementation(AFPSTemplateCharacter* Target, int Counter, int ShapeID,
	FVector Position, FQuat Rotation)
{
	Target->RollbackDebug->OnServerReceiveRemoteShape(OwnerCharacter, Counter, ShapeID, Position, Rotation);
}

void URollbackDebugComponent::ClientDisplayShapeTransform_Implementation(AFPSTemplateCharacter* Target, int ShapeID, 
	FVector Position, FQuat Rotation, ServerReplicationMessageType Type, float Duration)
{
	Target->RollbackDebug->DisplayShapeTransform(ShapeID, Position, Rotation, Type, Duration);
}

void URollbackDebugComponent::DisplayShapeTransform(int ShapeID, 
	FVector Position, FQuat Rotation, ServerReplicationMessageType Type, float Duration)
{
	physx::PxShape* Shape = OwnerCharacter->ShapeManager->GetAllShapes()[ShapeID];
	DebugUtil::DrawPxShape(GetWorld(), Shape, Position, Rotation, Type == ServerReplicationMessageType::ServerState ? FColor::Green : FColor::Yellow, Duration);
}

void URollbackDebugComponent::ServerSendShapeTransforms(AFPSTemplateCharacter* Target, ServerReplicationMessageType Type)
{
	for (int i = 0; i < Target->ShapeManager->GetAllShapes().Num(); i++) {
		PxRigidActor* PxActor = (PxRigidActor*) Target->ShapeManager->GetAllShapes()[i]->getActor();
		PxTransform ShapeGlobalPose = PxActor->getGlobalPose() * Target->ShapeManager->GetAllShapes()[i]->getLocalPose();
		ClientDisplayShapeTransform(Target, i, P2UVector(ShapeGlobalPose.p), P2UQuat(ShapeGlobalPose.q), Type, 4.0f);
	}
}

void URollbackDebugComponent::ClientSendRollbackShape_Implementation(AFPSTemplateCharacter* Target, int Counter, int ShapeID,
	FVector Position, FQuat Rotation)
{
	Target->RollbackDebug->OnClientReceiveRemoteShape(OwnerCharacter, Counter, ShapeID, Position, Rotation);
}

void URollbackDebugComponent::OnClientReceiveRemoteShape(AFPSTemplateCharacter* MonitoringPlayer, int Counter, int ShapeID,
	FVector Position, FQuat Rotation)
{
	while (PosesRemote.Num() < Counter + 1) {
		PosesRemote.Add(RepAnimationSnapshot(OwnerCharacter->ShapeManager->GetAllShapes()));
	}
	PxTransform Transform = PxTransform(U2PVector(Position), U2PQuat(Rotation));
	PosesRemote[Counter].SetShapeTransform(OwnerCharacter->ShapeManager->GetAllShapes()[ShapeID], Transform);

	if (PosesRemote[Counter].HasAddedAllTransforms()) {
		// Remote pose assembly complete!

		float HitRate = CalculateRandomHitRate(PositionsLocal[Counter], RotationsLocal[Counter], PosesLocal[Counter], PosesRemote[Counter]);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Hit rate: %f"), HitRate));
		MonitoringPlayer->RollbackDebug->RollbackLogger.LogDiscrepancy(LocalPoseTimes[Counter], HitRate, &PosesLocal[Counter], &PosesRemote[Counter]);
	}
}

void URollbackDebugComponent::OnServerReceiveRemoteShape(AFPSTemplateCharacter* MonitoringPlayer, int Counter, int ShapeID,
	FVector Position, FQuat Rotation)
{
	while (PosesRemote.Num() < Counter + 1) {
		PosesRemote.Add(RepAnimationSnapshot(OwnerCharacter->ShapeManager->GetAllShapes()));
	}
	PxTransform Transform = PxTransform(U2PVector(Position), U2PQuat(Rotation));
	PosesRemote[Counter].SetShapeTransform(OwnerCharacter->ShapeManager->GetAllShapes()[ShapeID], Transform);

	if (PosesRemote[Counter].HasAddedAllTransforms()) {
		// Remote pose assembly complete!

		if (LocalPoseTimes.Num() > 0) {
			float OptimalFudge, OptimalAngDiff, OptimalPosDiff;
			UCustomCharacterMovementComponent* MovementComponent = Cast<UCustomCharacterMovementComponent>(OwnerCharacter->GetCharacterMovement());
			FindOptimalRollbackFudge(Counter, OptimalFudge, OptimalAngDiff, OptimalPosDiff, MovementComponent->ReplicationType);
			//UE_LOG(LogGauntlet, Display, TEXT("Client %s"), *ClientSendTimes[Counter].ToString());
			//UE_LOG(LogGauntlet, Display, TEXT("Now %s"), *FDateTime::Now().ToString());
			float TransmissionTime = (FDateTime::Now() - ClientSendTimes[Counter]).GetTotalSeconds();
			MonitoringPlayer->RollbackDebug->ClientSendDebugOptimalFudge(GetWorld()->GetTimeSeconds(), OptimalFudge, OptimalAngDiff, OptimalPosDiff, MonitoringPlayer->GetPingRaw() * 0.001f/*TransmissionTime*/);
			UE_LOG(LogGauntlet, Display, TEXT("Ping at server: %f"), MonitoringPlayer->GetPingRaw());
		}
	}
}

void URollbackDebugComponent::ClientSendDebugOptimalFudge_Implementation(float Time, float OptimalFudge, float OptimalAngDiff, 
	float OptimalPosDiff, float TransmissionTime)
{
	RollbackLogger.LogOptimalFudge(Time, OptimalFudge, OptimalAngDiff, OptimalPosDiff, TransmissionTime);
	UE_LOG(LogGauntlet, Display, TEXT("Ping at client: %f"), OwnerCharacter->GetPingRaw());
}

void URollbackDebugComponent::FindOptimalRollbackFudge(int Counter, float& OptimalFudge, float& OptimalAngDiff, float& OptimalPosDiff, 
	MovementReplicationType ReplicationType)
{

	float Time = LocalPoseTimes[Counter];
	RepAnimationSnapshot ClientPose = PosesRemote[Counter];
	AFPSTemplateGameMode* GameMode = (AFPSTemplateGameMode *)GetWorld()->GetAuthGameMode();
	TMap<physx::PxShape*, physx::PxTransform> RollbackTransforms;
	OptimalFudge = 0.0f;


	if (GameMode != NULL) {
		if (GameMode->ShouldUseRollback) {
			GameMode->GetRepWorldTimelines().PreRollbackTarget((IRepMovable*)OwnerCharacter);
		}
		// Trisection method to find minimum

		int NumIterations = 15;
		float MinFudgeFactor = -0.1f;
		float MaxFudgeFactor = 0.1f;

		for (int i = 0; i < NumIterations; i++) 			
		{
			float LowerMid = MinFudgeFactor + (MaxFudgeFactor - MinFudgeFactor) / 3.0f;
			float UpperMid = MinFudgeFactor + 2.0f * (MaxFudgeFactor - MinFudgeFactor) / 3.0f;

			if (GameMode->ShouldUseRollback) {
				GameMode->GetRepWorldTimelines().RollbackTarget((IRepMovable*)OwnerCharacter, GetWorld()->GetTimeSeconds() + LowerMid,
					RepTimeline<RepSnapshot>::InterpolationOffset, OwnerCharacter->GetPing(), ReplicationType);
			}
			RollbackTransforms.Empty();
			OwnerCharacter->ShapeManager->SavePhysicsShapeTransformsGlobal(RollbackTransforms);
			RepAnimationSnapshot RollbackPose(RollbackTransforms);
			float AverageAngleDiscrepancyLower = RepAnimationSnapshot::GetAverageAngleDiscrepancy(ClientPose, RollbackPose);
			//UE_LOG(LogGauntlet, Display, TEXT("Lower ang: %f"), AverageAngleDiscrepancyLower);
			float AveragePositionDiscrepancyLower = RepAnimationSnapshot::GetAveragePositionDiscrepancy(ClientPose, RollbackPose);
			//UE_LOG(LogGauntlet, Display, TEXT("Lower pos: %f"), AveragePositionDiscrepancyLower);
			float DiscrepancyScoreLower = AverageAngleDiscrepancyLower + AveragePositionDiscrepancyLower;

			if (GameMode->ShouldUseRollback) {
				GameMode->GetRepWorldTimelines().RollbackTarget((IRepMovable*)OwnerCharacter, GetWorld()->GetTimeSeconds() + UpperMid,
					RepTimeline<RepSnapshot>::InterpolationOffset, OwnerCharacter->GetPing(), ReplicationType);
			}
			RollbackTransforms.Empty();
			OwnerCharacter->ShapeManager->SavePhysicsShapeTransformsGlobal(RollbackTransforms);
			RollbackPose = RepAnimationSnapshot(RollbackTransforms);
			float AverageAngleDiscrepancyUpper = RepAnimationSnapshot::GetAverageAngleDiscrepancy(ClientPose, RollbackPose);
			//UE_LOG(LogGauntlet, Display, TEXT("Upper ang: %f"), AverageAngleDiscrepancyUpper);
			float AveragePositionDiscrepancyUpper = RepAnimationSnapshot::GetAveragePositionDiscrepancy(ClientPose, RollbackPose);
			//UE_LOG(LogGauntlet, Display, TEXT("Upper pos: %f"), AveragePositionDiscrepancyUpper);
			float DiscrepancyScoreUpper = AverageAngleDiscrepancyUpper + AveragePositionDiscrepancyUpper;

			if (DiscrepancyScoreLower < DiscrepancyScoreUpper) {
				MaxFudgeFactor = UpperMid;
				OptimalAngDiff = AverageAngleDiscrepancyLower;
				OptimalPosDiff = AveragePositionDiscrepancyLower;
			} else {
				MinFudgeFactor = LowerMid;
				OptimalAngDiff = AverageAngleDiscrepancyUpper;
				OptimalPosDiff = AveragePositionDiscrepancyUpper;
			}
		}

		OptimalFudge = (MinFudgeFactor + MaxFudgeFactor) / 2.0f;

		if (GameMode->ShouldUseRollback) {
			GameMode->GetRepWorldTimelines().ResetTarget((IRepMovable*)OwnerCharacter);
		}
	}
}

void URollbackDebugComponent::RollbackDebugOffset(float Offset)
{
	ServerSetReplicationOffset(Offset);
}

void URollbackDebugComponent::ServerSetReplicationOffset_Implementation(float Offset)
{
	OwnerCharacter->RollbackOffset = Offset;
}

void URollbackDebugComponent::ServerSetLatency_Implementation(int Latency)
{
    FPacketSimulationSettings CustomSettings;
    CustomSettings.PktIncomingLagMin = Latency / 2;
    CustomSettings.PktIncomingLagMax = Latency / 2;
	CustomSettings.PktLag = Latency / 2;
    UWorld* World = GetWorld();
    if (World != NULL) {
        World->GetNetDriver()->SetPacketSimulationSettings(CustomSettings);
    }
}

//-----------------------------------------
// Rollback UI
//-----------------------------------------

void URollbackDebugComponent::SetRollbackTimelineValue(float Value)
{
	DebugShapeDisplayTime = Value;
}

void URollbackDebugComponent::OnOpenRollbackTimelineUI(URollbackTimelineWidget* Widget)
{
	RollbackTimelineWidget = Widget;
}

void URollbackDebugComponent::SetShouldUpdateTimelineSlider(bool ShouldUpdateSlider)
{
	ShouldUpdateTimelineSlider = ShouldUpdateSlider;
}

void URollbackDebugComponent::SetShouldInterpolateDebugPoses(bool ShouldInterpolatePoses)
{
	ShouldInterpolateDebugPoses = ShouldInterpolatePoses;
}

void URollbackDebugComponent::DebugSetShapeDisplayOffset(float Offset)
{
	DebugShapeDisplayOffset = Offset;
}

//-----------------------------------------
// Logging
//-----------------------------------------

void URollbackDebugComponent::SaveRollbackLog()
{
	RollbackLogger.DumpLogFile();
}

//-----------------------------------------
// Random Hit Rate tests
//-----------------------------------------

FVector URollbackDebugComponent::GetRandomPointInBoundingBox(FVector& CenterPosition, FQuat& CenterRotation) 
{
	float BoxHalfHeight = 100.0f;
	float BoxHalfWidth = 50.0f;
	FVector LocalUnrotated = UKismetMathLibrary::RandomPointInBoundingBox(FVector::ZeroVector, FVector(BoxHalfWidth, BoxHalfWidth, BoxHalfHeight));
	FVector Rotated = CenterRotation.RotateVector(LocalUnrotated);
	FVector Res = CenterPosition + Rotated;
	//DrawDebugPoint(GetWorld(), Res, 5.0f, FColor::Purple, false, 0.5f);
	return Res;
}

void URollbackDebugComponent::GenerateRandomBoundingBoxPositions(FVector& CenterPosition, FQuat& CenterRotation)
{
	RandomBoundingBoxPositions.Empty();
	for (int i = 0; i < NUM_RANDOM_HIT_TESTS; i++) {
		RandomBoundingBoxPositions.Add(GetRandomPointInBoundingBox(CenterPosition, CenterRotation));
	}
}

FRay URollbackDebugComponent::GetRandomCollisionTestRay(int RandomPointIndex)
{
	float Dist = 300.0f;
	FVector PassThroughPoint = RandomBoundingBoxPositions[RandomPointIndex];
	FVector Start = OwnerCharacter->GetActorRightVector() * Dist + PassThroughPoint;
	DrawDebugLine(GetWorld(), Start, Start - (OwnerCharacter->GetActorRightVector() * Dist * 2.0f), FColor::Purple, false, 0.5f);
	//DrawDebugPoint(GetWorld(), Start, 10.0f, FColor::Blue, false, 0.5f);
	return FRay(Start, -OwnerCharacter->GetActorRightVector());
}

bool URollbackDebugComponent::TestHitFromRay(const FRay& Ray)
{
	float Dist = 300.0f;
	FCollisionQueryParams TraceParams;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	Hit.bBlockingHit = true;
	PxScene* PScene = GetWorld()->GetPhysicsScene()->GetPxScene();
	
	// Make a loop to be able to detect multiple blocking objects
	while (Hit.Actor != OwnerCharacter && Hit.bBlockingHit == true) {
		if (GetWorld()->LineTraceSingleByChannel(Hit, Ray.Origin, Ray.Origin + Ray.Direction * Dist * 2.f, ECC_Visibility, TraceParams)) {
			TraceParams.AddIgnoredActor(Hit.Actor.Get());
			if (Hit.Actor != OwnerCharacter) continue;
			return true;
			/*
			FBodyInstance* HitBodyInstance = Hit.Component->GetBodyInstance(Hit.BoneName);
			TArray<FPhysicsShapeHandle> collisionShapes;
			PScene->lockRead();
			int32 numSyncShapes = HitBodyInstance->GetAllShapes_AssumesLocked(collisionShapes);
			PScene->unlockRead();
			for (int i = 0; i < numSyncShapes; i++) {
				if (AllShapes.Contains(collisionShapes[i].Shape)) {
					IsHit = true;
				}
			}
			if (IsHit) {
				break;
			}
			*/
		}
	}
	return false;
}

float URollbackDebugComponent::CalculateRandomHitRate(FVector& Position, FQuat& Rotation, RepAnimationSnapshot& LocalSnapshot, 
	RepAnimationSnapshot& RollbackSnapshot)
{
	int Matches = 0;
	int LocalHits = 0;
	RandomHitTestResults.Empty();
	GenerateRandomBoundingBoxPositions(Position, Rotation);

	TMap<physx::PxShape*, physx::PxTransform> ShapeTransforms;
	OwnerCharacter->ShapeManager->SavePhysicsShapeTransformsGlobal(ShapeTransforms);
	RepAnimationSnapshot OriginalSnapshot(ShapeTransforms);

	// First, test local snapshot, store the intermediate results
	OwnerCharacter->ApplyAnimationSnapshot(LocalSnapshot);

	for (int i = 0; i < NUM_RANDOM_HIT_TESTS; i++) {
		FRay Ray = GetRandomCollisionTestRay(i);
		RandomHitTestResults.Add(TestHitFromRay(Ray));
		if (RandomHitTestResults[i]) {
			LocalHits++;
		}
	}

	OwnerCharacter->ApplyAnimationSnapshot(RollbackSnapshot);

	for (int i = 0; i < NUM_RANDOM_HIT_TESTS; i++) {
		// Right now, do not test for false negatives
		if (!RandomHitTestResults[i]) continue;
		FRay Ray = GetRandomCollisionTestRay(i);
		if (TestHitFromRay(Ray)) {
			Matches++;
		}
	}

	OwnerCharacter->ApplyAnimationSnapshot(OriginalSnapshot);

	return (float) Matches / LocalHits;
}
