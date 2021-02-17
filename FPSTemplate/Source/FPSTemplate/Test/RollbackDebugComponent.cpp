// Fill out your copyright notice in the Description page of Project Settings.


#include "Test/RollbackDebugComponent.h"
#include "Runtime/Engine/Public/EngineGlobals.h"
#include "Kismet/KismetMathLibrary.h"
#include "Core/DebugUtil.h"
#include "Core/FPSTemplateCharacter.h"
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
	
	IsUsingDebugMovement = false;
}

void URollbackDebugComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

// Called every frame
void URollbackDebugComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsUsingDebugMovement)
	{
		OwnerCharacter->MoveForward(FMath::Sin(GetWorld()->GetTimeSeconds()));
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
			PxTransform& LocalTransform = LocalPose.GetShapeTransforms()[Shape];
			DebugUtil::DrawPxShape(GetWorld(), Shape, P2UVector(LocalTransform.p), P2UQuat(LocalTransform.q), FColor::Yellow, 0.0f);

			PxTransform& RollbackTransform = RollbackPose.GetShapeTransforms()[Shape];
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
		if (GetWorld()->GetTimeSeconds() - LastDebugShapeSendTime > 0.4f && DebugIsMonitoring) {
			AFPSTemplateCharacter* OtherPlayer = DebugFindOtherPlayer();
			if (OtherPlayer != NULL) {
				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Ping at client: %f"), GetPingRaw()));
				ServerRequestAnimState(OtherPlayer, OtherPlayer->RollbackDebug->AnimSaveCounter);
				OtherPlayer->RollbackDebug->SaveLocalShapeForDebug();
			} 
			LastDebugShapeSendTime = GetWorld()->GetTimeSeconds();
		}
	}
}

void URollbackDebugComponent::SaveLocalShapeForDebug()
{
	TMap<physx::PxShape*, physx::PxTransform> ShapeTransforms;
	OwnerCharacter->ShapeManager->SavePhysicsShapeTransformsGlobal(ShapeTransforms);
	PosesLocal.Add(RepAnimationSnapshot(ShapeTransforms));
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

void URollbackDebugComponent::StartDebugMovement()
{
    UE_LOG(LogGauntlet, Display, TEXT("Start debug movement"));
	if (OwnerCharacter->HasLocalNetOwner()) {
		UE_LOG(LogGauntlet, Display, TEXT("Is using debug movement!"));
		DebugMovementStartTime = GetWorld()->GetTimeSeconds();
		IsUsingDebugMovement = true;
	}
	else {
		UE_LOG(LogGauntlet, Display, TEXT("NO LOCAL NET OWNER!"));
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

void URollbackDebugComponent::DebugPrepareMonitoringTest()
{
	// Player starts are randomly assigned, so for consistency, set the start position
	ServerSetInitialTransform(FVector(0.0f, -300.0f, 262.0f), FRotationMatrix::MakeFromX(FVector(1.0f, 0.0f, 0.0f)).ToQuat());
}

void URollbackDebugComponent::DebugStartMonitoring()
{
	DebugIsMonitoring = true;
	LastDebugShapeSendTime = GetWorld()->GetTimeSeconds();
	IsScoping = true;
	DebugFindOtherPlayer()->RollbackDebug->RollbackLogger.CreateLogFile();
}

//-----------------------------------------
// RPCS
//-----------------------------------------

void URollbackDebugComponent::ServerSetInitialTransform_Implementation(FVector Position, FQuat Rotation)
{
	OwnerCharacter->TeleportTo(Position, Rotation.Rotator());
}

void URollbackDebugComponent::ServerRequestAnimState_Implementation(AFPSTemplateCharacter* Target, int Counter)
{
	AFPSTemplateGameMode* GameMode = (AFPSTemplateGameMode *)GetWorld()->GetAuthGameMode();
	if (GameMode != NULL) {
		GameMode->GetRepWorldTimelines().PreRollbackTarget((IRepMovable*) Target), 
		GameMode->GetRepWorldTimelines().RollbackTarget((IRepMovable*) Target, GetWorld()->GetTimeSeconds(), 
			RepTimeline<RepSnapshot>::InterpolationOffset, Target->GetPing());
		for (int i = 0; i < Target->ShapeManager->GetAllShapes().Num(); i++) {
			PxRigidActor* PxActor = (PxRigidActor*) Target->ShapeManager->GetAllShapes()[i]->getActor();
			PxTransform ShapeGlobalPose = PxActor->getGlobalPose() * Target->ShapeManager->GetAllShapes()[i]->getLocalPose();
			ClientSendRollbackShape(Target, Counter, i, P2UVector(ShapeGlobalPose.p), P2UQuat(ShapeGlobalPose.q));
		}
		GameMode->GetRepWorldTimelines().ResetTarget((IRepMovable*) Target);
	}
}

void URollbackDebugComponent::ServerStartLocalShapeTransmission_Implementation(AFPSTemplateCharacter* Target, int Counter)
{
	while (LocalPoseTimes.Num() < Counter + 1) {
		LocalPoseTimes.Add(0.0f);
	}
	LocalPoseTimes[Counter] = GetWorld()->GetTimeSeconds();
}

void URollbackDebugComponent::ServerSendLocalShape_Implementation(AFPSTemplateCharacter* Target, int Counter, int ShapeID,
	FVector Position, FQuat Rotation)
{
	Target->RollbackDebug->OnReceiveRemoteShape(Counter, ShapeID, Position, Rotation);
}

void URollbackDebugComponent::ClientDisplayShapeTransform_Implementation(AFPSTemplateCharacter* Target, int ShapeID, 
	FVector Position, FQuat Rotation, ServerReplicationMessageType Type)
{
	Target->RollbackDebug->DisplayShapeTransform(ShapeID, Position, Rotation, Type);
}

void URollbackDebugComponent::DisplayShapeTransform(int ShapeID, 
	FVector Position, FQuat Rotation, ServerReplicationMessageType Type)
{
	physx::PxShape* Shape = OwnerCharacter->ShapeManager->GetAllShapes()[ShapeID];
	DebugUtil::DrawPxShape(GetWorld(), Shape, Position, Rotation, Type == ServerReplicationMessageType::ServerState ? FColor::Green : FColor::Yellow, 0.0f);
}

void URollbackDebugComponent::ServerSendShapeTransforms(AFPSTemplateCharacter* Target, ServerReplicationMessageType Type)
{
	for (int i = 0; i < Target->ShapeManager->GetAllShapes().Num(); i++) {
		PxRigidActor* PxActor = (PxRigidActor*) Target->ShapeManager->GetAllShapes()[i]->getActor();
		PxTransform ShapeGlobalPose = PxActor->getGlobalPose() * Target->ShapeManager->GetAllShapes()[i]->getLocalPose();
		ClientDisplayShapeTransform(Target, i, P2UVector(ShapeGlobalPose.p), P2UQuat(ShapeGlobalPose.q), Type);
	}
}

void URollbackDebugComponent::ClientSendRollbackShape_Implementation(AFPSTemplateCharacter* Target, int Counter, int ShapeID,
	FVector Position, FQuat Rotation)
{
	Target->RollbackDebug->OnReceiveRemoteShape(Counter, ShapeID, Position, Rotation);
}

void URollbackDebugComponent::OnReceiveRemoteShape(int Counter, int ShapeID,
	FVector Position, FQuat Rotation)
{
	while (PosesRemote.Num() < Counter + 1) {
		PosesRemote.Add(RepAnimationSnapshot(OwnerCharacter->ShapeManager->GetAllShapes()));
	}
	PxTransform Transform = PxTransform(U2PVector(Position), U2PQuat(Rotation));
	PosesRemote[Counter].SetShapeTransform(OwnerCharacter->ShapeManager->GetAllShapes()[ShapeID], Transform);

	if (PosesRemote[Counter].HasAddedAllTransforms()) {
		// Remote pose assembly complete!

		FindOptimalRollbackFudge(Counter);
		/*
		float HitRate = CalculateRandomHitRate(PosesRollback[Counter]);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("Hit rate: %f"), HitRate));
		RollbackLogger.LogDiscrepancy(LocalPoseTimes[Counter], HitRate, &PosesLocal[Counter], &PosesRollback[Counter]);
		UE_LOG(LogTemp, Warning, TEXT("Actor: %s"), *GetName());
		*/
	}
}

void URollbackDebugComponent::FindOptimalRollbackFudge(int Counter)
{
	float Time = LocalPoseTimes[Counter];
	RepAnimationSnapshot ClientPose = PosesRemote[Counter];
	AFPSTemplateGameMode* GameMode = (AFPSTemplateGameMode *)GetWorld()->GetAuthGameMode();
	if (GameMode != NULL) {
		GameMode->GetRepWorldTimelines().PreRollbackTarget((IRepMovable*) OwnerCharacter),
		// REPEAT
		GameMode->GetRepWorldTimelines().RollbackTarget((IRepMovable*) OwnerCharacter, GetWorld()->GetTimeSeconds(),
			RepTimeline<RepSnapshot>::InterpolationOffset, OwnerCharacter->GetPing());
		// LOG BEST
		GameMode->GetRepWorldTimelines().ResetTarget((IRepMovable*)OwnerCharacter);
	}
}

void URollbackDebugComponent::RollbackDebugOffset(float Offset)
{
	ServerSetReplicationOffset(Offset);
	UE_LOG(LogTemp, Warning, TEXT("Bont rollback offset: %f"), OwnerCharacter->RollbackOffset);
}

void URollbackDebugComponent::ServerSetReplicationOffset_Implementation(float Offset)
{
	OwnerCharacter->RollbackOffset = Offset;
	UE_LOG(LogTemp, Warning, TEXT("Set rollback offset: %f"), OwnerCharacter->RollbackOffset);
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

FVector URollbackDebugComponent::GetRandomPointInBoundingBox() 
{
	float BoxHalfHeight = 100.0f;
	float BoxHalfWidth = 50.0f;
	FVector LocalUnrotated = UKismetMathLibrary::RandomPointInBoundingBox(FVector::ZeroVector, FVector(BoxHalfWidth, BoxHalfWidth, BoxHalfHeight));
	FVector Rotated = OwnerCharacter->GetActorQuat().RotateVector(LocalUnrotated);
	FVector Res = OwnerCharacter->GetActorLocation() + Rotated;
	//DrawDebugPoint(GetWorld(), Res, 5.0f, FColor::Purple, false, 0.5f);
	return Res;
}

void URollbackDebugComponent::GenerateRandomBoundingBoxPositions()
{
	RandomBoundingBoxPositions.Empty();
	for (int i = 0; i < NUM_RANDOM_HIT_TESTS; i++) {
		RandomBoundingBoxPositions.Add(GetRandomPointInBoundingBox());
	}
}

FRay URollbackDebugComponent::GetRandomCollisionTestRay(int RandomPointIndex)
{
	float Dist = 300.0f;
	FVector PassThroughPoint = RandomBoundingBoxPositions[RandomPointIndex];
	FVector Start = OwnerCharacter->GetActorRightVector() * Dist + PassThroughPoint;
	//DrawDebugLine(GetWorld(), Start, Start - (GetActorRightVector() * Dist * 2.0f), FColor::Purple, false, 0.5f);
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

float URollbackDebugComponent::CalculateRandomHitRate(RepAnimationSnapshot& RollbackSnapshot)
{
	int Matches = 0;
	int LocalHits = 0;
	RandomHitTestResults.Empty();
	GenerateRandomBoundingBoxPositions();

	// First, test without rollback, store the intermediate results
	for (int i = 0; i < NUM_RANDOM_HIT_TESTS; i++) {
		FRay Ray = GetRandomCollisionTestRay(i);
		RandomHitTestResults.Add(TestHitFromRay(Ray));
		if (RandomHitTestResults[i]) {
			LocalHits++;
		}
	}

	TMap<physx::PxShape*, physx::PxTransform> ShapeTransforms;
	OwnerCharacter->ShapeManager->SavePhysicsShapeTransformsGlobal(ShapeTransforms);
	RepAnimationSnapshot OriginalSnapshot(ShapeTransforms);

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

