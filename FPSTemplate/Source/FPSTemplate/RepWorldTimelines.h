// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RepMovable.h"
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
	virtual RepMovementTimeline& GetMovementTimeline(IRepMovable* RepMovable);
	virtual RepAnimationTimeline& GetAnimationTimeline(IRepMovable* RepMovable);
	virtual void PreRollbackWorld(IRepMovable* ExcludedMovable);
	virtual void RollbackWorld(IRepMovable* ExcludedMovable, float CurrentTime, float InterpolationOffset, float RTT);
	virtual void ResetWorld(IRepMovable* ExcludedMovable);
protected:
	TArray<IRepMovable*> RepObjects;
	TMap<IRepMovable*, RepMovementTimeline> MovementTimelines;
	TMap<IRepMovable*, RepAnimationTimeline> AnimationTimelines;
};
