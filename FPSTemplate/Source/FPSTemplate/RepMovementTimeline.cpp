// Fill out your copyright notice in the Description page of Project Settings.

#include "RepMovementTimeline.h"

RepMovementTimeline::RepMovementTimeline()
{
}

RepMovementTimeline::~RepMovementTimeline()
{
}

// TODO: Make the interpolation offset dynamic
const float RepMovementTimeline::InterpolationOffset = 0.8f;

void RepMovementTimeline::DeleteOldSnapshots(float Time)
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

void RepMovementTimeline::AddSnapshot(FVector Position, FQuat Rotation, FVector Velocity, float Time)
{
	Snapshots.Emplace(Position, Rotation, Velocity);
	SnapshotTimes.Add(Time);
}

bool RepMovementTimeline::HasSnapshots() const
{
	return Snapshots.Num() > 0;
}

RepSnapshot RepMovementTimeline::GetSnapshot(float Time) const
{
	if (SnapshotTimes.Num() == 0) {
		// There is no data, so just return a zeroed snapshot
		return RepSnapshot();
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
	
	// Find two snapshots and return an interpolation between them
	for (int i = 0; i < SnapshotTimes.Num(); i++) {
		//UE_LOG(LogTemp, Warning, TEXT("Snapshot at: %f"), SnapshotTimes[i]);
		if (SnapshotTimes[i] > Time) {
			// We found the first snapshot with a later time
			float InterpolationAlpha = UKismetMathLibrary::NormalizeToRange(Time, SnapshotTimes[i - 1], SnapshotTimes[i]);
			return Interpolate(Snapshots[i - 1], Snapshots[i], InterpolationAlpha);
		}
	}

	if (Time > SnapshotTimes[0]) {
		UE_LOG(LogTemp, Error, TEXT("Knas!"), Time);
	}

	// This line should never be reached
	checkNoEntry();

	return RepSnapshot();
}

RepSnapshot RepMovementTimeline::Interpolate(const RepSnapshot& Start, const RepSnapshot& End, float Alpha) const
{
	RepSnapshot InterpolatedSnapshot;
	InterpolatedSnapshot.Position = FMath::LerpStable(Start.Position, End.Position, Alpha);
	InterpolatedSnapshot.Rotation = FQuat::Slerp(Start.Rotation, End.Rotation, Alpha);
	InterpolatedSnapshot.Velocity = FMath::LerpStable(Start.Velocity, End.Velocity, Alpha);

	return InterpolatedSnapshot;
}
