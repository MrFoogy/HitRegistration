// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RepSnapshot.h"
#include "RepAnimationSnapshot.h"
#include "RepViewRotationSnapshot.h"
#include "Kismet/KismetMathLibrary.h"
#include "CoreMinimal.h"

/**
 * 
 */
template <typename T>
class FPSTEMPLATE_API RepTimeline
{
public:
	RepTimeline();
	~RepTimeline();

	/**
	* Calculates and returns a new snapshot for the given time using interpolation/extrapolation
	* @param Time - the time at which to calculate the snapshot
	*/
	T GetSnapshot(float Time, bool Print = false) const;

	/**
	* Removes all snapshots older than Time
	* @param Time - all snapshots older than this time will be removed
	*/
	void DeleteOldSnapshots(float Time);

	/**
	* Adds a new snapshot at the given Time
	*/
	void AddSnapshot(T NewSnapshot, float Time);

	/**
	* Returns whether or not the timeline contains any snapshots
	*/
	bool HasSnapshots() const;

	// Constant interpolation offset added
	static const float InterpolationOffset;

private:

	// List of snapshots sorted by time. At each position in this array, the array SnapshotTimes contains the corresponding time
	TArray<T> Snapshots;

	// List of times for the snapshots stored in the Snapshots array
	TArray<float> SnapshotTimes;

	/**
	* Calculates a new snapshot by linear interpolation
	*/
	T Interpolate(const T& Start, const T& End, float Alpha) const;
};
