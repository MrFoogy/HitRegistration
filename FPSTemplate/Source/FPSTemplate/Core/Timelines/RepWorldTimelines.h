// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Timelines/RepMovable.h"
#include "Core/Timelines/RepTimeline.h"
#include "Core/Timelines/RepSnapshot.h"
#include "Core/Timelines/RepAnimationSnapshot.h"

/**
 * 
 */
class FPSTEMPLATE_API URepWorldTimelines
{
public:
	URepWorldTimelines();
	virtual ~URepWorldTimelines();

	virtual void ClearWorldTimelines();
	virtual void AddRepObject(IRepMovable* RepMovable);
	virtual void RemoveRepObject(IRepMovable* RepObject);
	virtual void CreateMovementTimeline(IRepMovable* RepMovable);
	virtual void CreateAnimationTimeline(IRepMovable* RepMovable);

	virtual bool HasMovementTimeline(IRepMovable* RepMovable) const;
	virtual bool HasAnimationTimeline(IRepMovable* RepMovable) const;
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
