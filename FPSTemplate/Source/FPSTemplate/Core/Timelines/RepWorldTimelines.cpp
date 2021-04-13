// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Timelines/RepWorldTimelines.h"

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

void URepWorldTimelines::AddRepObject(IRepMovable* RepMovable)
{
	RepObjects.Add(RepMovable);
}

void URepWorldTimelines::RemoveRepObject(IRepMovable* RepObject)
{
	RepObjects.Remove(RepObject);
	MovementTimelines.Remove(RepObject);
	AnimationTimelines.Remove(RepObject);
}

void URepWorldTimelines::CreateMovementTimeline(IRepMovable* RepMovable)
{
	MovementTimelines.Add(RepMovable, RepTimeline<RepSnapshot>());
}

void URepWorldTimelines::CreateAnimationTimeline(IRepMovable* RepMovable)
{
	AnimationTimelines.Add(RepMovable, RepTimeline<RepAnimationSnapshot>());
}

bool URepWorldTimelines::HasMovementTimeline(IRepMovable* RepMovable) const
{
	return MovementTimelines.Contains(RepMovable);
}

bool URepWorldTimelines::HasAnimationTimeline(IRepMovable* RepMovable) const
{
	return AnimationTimelines.Contains(RepMovable);
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

void URepWorldTimelines::RollbackWorld(IRepMovable* ExcludedMovable, float CurrentTime, float InterpolationOffset, float RTT, 
	MovementReplicationType ReplicationType)
{
	for (IRepMovable* RepObject : RepObjects) {
		if (RepObject != ExcludedMovable) {
			RollbackTarget(RepObject, CurrentTime, InterpolationOffset, RTT, ReplicationType);
		}
	}
}

void URepWorldTimelines::RollbackTarget(IRepMovable* TargetMovable, float CurrentTime, float InterpolationOffset, float RTT, 
	MovementReplicationType ReplicationType)
{
	float RollbackTime;
	if (ReplicationType == MovementReplicationType::Interpolation) {
		RollbackTime = CurrentTime - InterpolationOffset - RTT;
	}
	else if (ReplicationType == MovementReplicationType::Default) {
		RollbackTime = CurrentTime - RTT;
	}

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

