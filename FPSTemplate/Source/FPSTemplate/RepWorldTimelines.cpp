// Fill out your copyright notice in the Description page of Project Settings.


#include "RepWorldTimelines.h"

URepWorldTimelines::URepWorldTimelines()
{
}

URepWorldTimelines::~URepWorldTimelines()
{
}

void URepWorldTimelines::ClearWorldTimelines()
{
	MovementTimelines.Empty();
	AnimationTimelines.Empty();
}

void URepWorldTimelines::InitializeTimeline(IRepMovable* RepMovable)
{
	RepObjects.Add(RepMovable);
	MovementTimelines.Add(RepMovable, RepTimeline<RepSnapshot>());
	AnimationTimelines.Add(RepMovable, RepTimeline<RepAnimationSnapshot>());
}

bool URepWorldTimelines::HasTimeline(IRepMovable* RepMovable) const
{
	return MovementTimelines.Contains(RepMovable);
}

RepTimeline<RepSnapshot>& URepWorldTimelines::GetMovementTimeline(IRepMovable* RepMovable)
{
	return MovementTimelines[RepMovable];
}

RepTimeline<RepAnimationSnapshot>& URepWorldTimelines::GetAnimationTimeline(IRepMovable* RepMovable)
{
	return AnimationTimelines[RepMovable];
}

void URepWorldTimelines::PreRollbackWorld(IRepMovable* ExcludedMovable)
{
	for (IRepMovable* RepObject : RepObjects) 
	{
		if (RepObject == ExcludedMovable) continue;
		PreRollbackTarget(RepObject);
	}
}

void URepWorldTimelines::PreRollbackTarget(IRepMovable* TargetMovable)
{
	TargetMovable->PrepareRollback();
}

void URepWorldTimelines::RollbackWorld(IRepMovable* ExcludedMovable, float CurrentTime, float InterpolationOffset, float RTT)
{
	for (IRepMovable* RepObject : RepObjects) {
		if (RepObject != ExcludedMovable) {
			RollbackTarget(RepObject, CurrentTime, InterpolationOffset, RTT);
		}
	}
}

void URepWorldTimelines::RollbackTarget(IRepMovable* TargetMovable, float CurrentTime, float InterpolationOffset, float RTT)
{
	float RollbackTime = CurrentTime - InterpolationOffset - RTT;

	if (MovementTimelines.Contains(TargetMovable)) {
		RepSnapshot RollbackSnapshot = MovementTimelines[TargetMovable].GetSnapshot(RollbackTime);
		TargetMovable->RollbackMovement(RollbackSnapshot);
	}
	if (AnimationTimelines.Contains(TargetMovable)) {
		RepAnimationSnapshot RollbackSnapshot = AnimationTimelines[TargetMovable].GetSnapshot(RollbackTime);
		TargetMovable->RollbackAnimation(RollbackSnapshot);
	}
}

void URepWorldTimelines::ResetWorld(IRepMovable* ExcludedMovable)
{
	for (IRepMovable* RepObject : RepObjects) {
		if (RepObject == ExcludedMovable) continue;
		ResetTarget(RepObject);
	}
}

void URepWorldTimelines::ResetTarget(IRepMovable* TargetMovable)
{
	TargetMovable->ResetRollback();
}
