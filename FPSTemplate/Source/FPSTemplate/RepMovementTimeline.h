// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/KismetMathLibrary.h"
#include "RepSnapshot.h"
#include "CoreMinimal.h"

/**
 * 
 */
class FPSTEMPLATE_API RepMovementTimeline
{
public:
	RepMovementTimeline();
	~RepMovementTimeline();

	/**
	* Calculates and returns a new snapshot for the given time using interpolation/extrapolation
	* @param Time - the time at which to calculate the snapshot
	*/
	RepSnapshot GetSnapshot(float Time) const;

	/**
	* Removes all snapshots older than Time
	* @param Time - all snapshots older than this time will be removed
	*/
	void DeleteOldSnapshots(float Time);

	/**
	* Adds a new snapshot at the given Time
	*/
	void AddSnapshot(FVector Position, FQuat Rotation, FVector Velocity, float Time);

	/**
	* Returns whether or not the timeline contains any snapshots
	*/
	bool HasSnapshots() const;

	// Constant interpolation offset added
	static const float InterpolationOffset;

private:

	// List of snapshots sorted by time. At each position in this array, the array SnapshotTimes contains the corresponding time
	TArray<RepSnapshot> Snapshots;

	// List of times for the snapshots stored in the Snapshots array
	TArray<float> SnapshotTimes;

	/**
	* Calculates a new snapshot by linear interpolation
	*/
	RepSnapshot Interpolate(const RepSnapshot& Start, const RepSnapshot& End, float Alpha) const;
};
