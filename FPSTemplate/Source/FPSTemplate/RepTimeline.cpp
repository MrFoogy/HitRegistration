// Fill out your copyright notice in the Description page of Project Settings.


#include "RepTimeline.h"

template <typename T>
RepTimeline<T>::RepTimeline()
{
}

template <typename T>
RepTimeline<T>::~RepTimeline()
{
}

// TODO: Make the interpolation offset dynamic
template <typename T>
const float RepTimeline<T>::InterpolationOffset = 0.8f;

template <typename T>
void RepTimeline<T>::DeleteOldSnapshots(float Time)
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

template <typename T>
void RepTimeline<T>::AddSnapshot(T NewSnapshot, float Time)
{
	Snapshots.Add(NewSnapshot);
	SnapshotTimes.Add(Time);
}

template <typename T>
bool RepTimeline<T>::HasSnapshots() const
{
	return Snapshots.Num() > 0;
}

template <typename T>
T RepTimeline<T>::GetSnapshot(float Time, bool Print) const
{
	if (SnapshotTimes.Num() == 0) {
		// There is no data, so just return a zeroed snapshot
		return T();
	}
	if (Time <= SnapshotTimes[0]) {
		// The requested time predates the timeline, so simply return the first snapshot
		return Snapshots[0];
	}
	if (Time >= SnapshotTimes.Last()) {
		// The requested time postdates the timeline, so simply return the last snapshot
		//TODO: add extrapolation here?
		return Snapshots.Last();
	}
	
	if (Print) UE_LOG(LogTemp, Warning, TEXT("Snapshots: %d"), SnapshotTimes.Num());
	// Find two snapshots and return an interpolation between them
	for (int i = 0; i < SnapshotTimes.Num(); i++) {
		//UE_LOG(LogTemp, Warning, TEXT("Snapshot at: %f"), SnapshotTimes[i]);
		if (SnapshotTimes[i] > Time) {
			// We found the first snapshot with a later time
			if (Print) UE_LOG(LogTemp, Warning, TEXT("Use: %d"), i);
			float InterpolationAlpha = UKismetMathLibrary::NormalizeToRange(Time, SnapshotTimes[i - 1], SnapshotTimes[i]);
			return Interpolate(Snapshots[i - 1], Snapshots[i], InterpolationAlpha);
		}
	}

	if (Time > SnapshotTimes[0]) {
		UE_LOG(LogTemp, Error, TEXT("Knas!"), Time);
	}

	// This line should never be reached
	checkNoEntry();

	return T();
}

template <typename T>
T RepTimeline<T>::Interpolate(const T& Start, const T& End, float Alpha) const
{
	return T::Interpolate(Start, End, Alpha);
}

// Add definitions here to avoid linker errors
// See: https://isocpp.org/wiki/faq/templates#separate-template-class-defn-from-decl
template class RepTimeline<RepSnapshot>;
template class RepTimeline<RepAnimationSnapshot>;
template class RepTimeline<RepViewRotationSnapshot>;
