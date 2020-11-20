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
	* Adds a new snapshot at the given Time
	*/
	void AddSnapshotCompensating(T NewSnapshot, float Time, float CurrentInterpolationTime, float UpdateFrequency);

	/**
	* Checks if is currently not interpolating, meaning there was a long time since the last update. 
	* If so, insert a duplicate of the latest snapshot to emulate receiving another recent update,
	* at a Time determined by UpdateFrequency
	*/
	void CompensateMissedUpdates(float InterpolationTime, float CurrentTime, float UpdateFrequency);

	/**
	* Returns whether or not the timeline contains any snapshots
	*/
	bool HasSnapshots() const;

	/**
	* Returns whether or not the timeline has a past and future snapshot to interpolate between at the given time
	* @param Time - the time at which to check for past and future snapshots
	*/
	bool IsInInterpolationTime(float Time) const;

	/**
	* Update the time of the last snapshot to be Time
	*/
	void UpdateLastSnapshotTime(float Time);

	/**
	* Duplicates the last snapshot and adds it to the timeline at time Time.
	*/
	void AddDuplicateLastSnapshot(float Time);

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
