// Fill out your copyright notice in the Description page of Project Settings.

#include "RepAnimationTimeline.h"

RepAnimationTimeline::RepAnimationTimeline()
{
}

RepAnimationTimeline::~RepAnimationTimeline()
{
}

// TODO: Make the interpolation offset dynamic
const float RepAnimationTimeline::InterpolationOffset = 0.1f;

void RepAnimationTimeline::DeleteOldSnapshots(float Time)
{
	int NumSnapshotsToDelete = 0;

	// The snapshots are sorted by time, so simply iterate until we find the first newer snapshot
	for (int i = 0; i < SnapshotTimes.Num(); i++) {
		if (SnapshotTimes[i] > Time) {
			NumSnapshotsToDelete = i;
			break;
		}
	}

	// Detect the case where all snapshots were older
	if (SnapshotTimes.Num() > 0 && SnapshotTimes.Last() < Time) {
		NumSnapshotsToDelete = SnapshotTimes.Num();
	}

	if (NumSnapshotsToDelete > 0) {
		Snapshots.RemoveAt(0, NumSnapshotsToDelete);
		SnapshotTimes.RemoveAt(0, NumSnapshotsToDelete);
	}
}

void RepAnimationTimeline::AddSnapshot(const TMap<physx::PxShape*, physx::PxTransform>& ShapeTransforms, float Time)
{
	Snapshots.Emplace(ShapeTransforms);
	SnapshotTimes.Add(Time);
}

bool RepAnimationTimeline::HasSnapshots() const
{
	return Snapshots.Num() > 0;
}

RepAnimationSnapshot RepAnimationTimeline::GetSnapshot(float Time) const
{
	if (SnapshotTimes.Num() == 0) {
		// There is no data, so just return a zeroed snapshot
		return RepAnimationSnapshot();
	}
	if (Time < SnapshotTimes[0]) {
		// The requested time predates the timeline, so simply return the first snapshot
		return Snapshots[0];
	}
	if (Time > SnapshotTimes.Last()) {
		// The requested time postdates the timeline, so simply return the last snapshot
		//TODO: add extrapolation here?
		return Snapshots.Last();
	}

	//UE_LOG(LogTemp, Warning, TEXT("We're properly interpolating!"));
	
	//UE_LOG(LogTemp, Warning, TEXT("Total snapshots: %d"), Snapshots.Num());
	// Find two snapshots and return an interpolation between them
	for (int i = 0; i < SnapshotTimes.Num(); i++) {
		//UE_LOG(LogTemp, Warning, TEXT("Snapshot at: %f"), SnapshotTimes[i]);
		if (SnapshotTimes[i] > Time) {
			// We found the first snapshot with a later time
			//UE_LOG(LogTemp, Warning, TEXT("Used snapshot: %d"), i);
			float InterpolationAlpha = UKismetMathLibrary::NormalizeToRange(Time, SnapshotTimes[i - 1], SnapshotTimes[i]);
			return Interpolate(Snapshots[i - 1], Snapshots[i], InterpolationAlpha);
		}
	}

	if (Time > SnapshotTimes[0]) {
		UE_LOG(LogTemp, Error, TEXT("Knas!"), Time);
	}

	// This line should never be reached
	checkNoEntry();

	return RepAnimationSnapshot();
}

RepAnimationSnapshot RepAnimationTimeline::Interpolate(const RepAnimationSnapshot& Start, const RepAnimationSnapshot& End, float Alpha) const
{
	RepAnimationSnapshot InterpolatedSnapshot;
	for (auto& StartElem : Start.ShapeTransforms) {
		physx::PxShape* Shape = StartElem.Key;
		const physx::PxTransform& StartTransform = StartElem.Value;
		const physx::PxTransform& EndTransform = End.ShapeTransforms[Shape];
		physx::PxVec3 PxPosition = ((StartTransform.p * (1.0f - Alpha)) + (EndTransform.p * Alpha));
		physx::PxQuat PxRotation = U2PQuat(FQuat::FastLerp(P2UQuat(StartTransform.q), P2UQuat(EndTransform.q), Alpha));
		InterpolatedSnapshot.SetShapeTransform(Shape, physx::PxTransform(PxPosition, PxRotation));
	}

	return InterpolatedSnapshot;
}
