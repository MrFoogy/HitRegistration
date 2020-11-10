// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/KismetMathLibrary.h"
#include "RepAnimationSnapshot.h"
#include "PhysXIncludes.h"
#include "PhysicsPublic.h"	
#include "Runtime/Engine/Private/PhysicsEngine/PhysXSupport.h"
#include "CoreMinimal.h"

/**
 * 
 */
class FPSTEMPLATE_API RepAnimationTimeline
{
public:
	RepAnimationTimeline();
	~RepAnimationTimeline();

	/**
	* Calculates and returns a new snapshot for the given time using interpolation/extrapolation
	* @param Time - the time at which to calculate the snapshot
	*/
	RepAnimationSnapshot GetSnapshot(float Time) const;

	/**
	* Removes all snapshots older than Time
	* @param Time - all snapshots older than this time will be removed
	*/
	void DeleteOldSnapshots(float Time);

	/**
	* Adds a new snapshot at the given Time
	*/
	void AddSnapshot(const TMap<physx::PxShape*, physx::PxTransform>& ShapeTransforms, float Time);

	/**
	* Returns whether or not the timeline contains any snapshots
	*/
	bool HasSnapshots() const;

	// Constant interpolation offset added
	static const float InterpolationOffset;

private:

	// List of snapshots sorted by time. At each position in this array, the array SnapshotTimes contains the corresponding time
	TArray<RepAnimationSnapshot> Snapshots;

	// List of times for the snapshots stored in the Snapshots array
	TArray<float> SnapshotTimes;

	/**
	* Calculates a new snapshot by linear interpolation
	*/
	RepAnimationSnapshot Interpolate(const RepAnimationSnapshot& Start, const RepAnimationSnapshot& End, float Alpha) const;
};
