// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RepMovable.h"
#include "RepTimeline.h"
#include "RepSnapshot.h"
#include "RepAnimationSnapshot.h"
#include "RepMovementTimeline.h"
#include "RepAnimationTimeline.h"

/**
 * 
 */
class FPSTEMPLATE_API URepWorldTimelines
{
public:
	URepWorldTimelines();
	virtual ~URepWorldTimelines();

	virtual void ClearWorldTimelines();
	virtual void InitializeTimeline(IRepMovable* RepMovable);
	virtual bool HasTimeline(IRepMovable* RepMovable) const;
	virtual RepTimeline<RepSnapshot>& GetMovementTimeline(IRepMovable* RepMovable);
	virtual RepTimeline<RepAnimationSnapshot>& GetAnimationTimeline(IRepMovable* RepMovable);
	virtual void PreRollbackWorld(IRepMovable* ExcludedMovable);
	virtual void PreRollbackTarget(IRepMovable* TargetMovable);
	virtual void RollbackWorld(IRepMovable* ExcludedMovable, float CurrentTime, float InterpolationOffset, float RTT);
	virtual void RollbackTarget(IRepMovable* TargetMovable, float CurrentTime, float InterpolationOffset, float RTT);
	virtual void ResetWorld(IRepMovable* ExcludedMovable);
	virtual void ResetTarget(IRepMovable* TargetMovable);
protected:
	TArray<IRepMovable*> RepObjects;
	TMap<IRepMovable*, RepTimeline<RepSnapshot>> MovementTimelines;
	TMap<IRepMovable*, RepTimeline<RepAnimationSnapshot>> AnimationTimelines;
};
