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
		RepObject->PrepareRollback();
	}
}

void URepWorldTimelines::RollbackWorld(IRepMovable* ExcludedMovable, float CurrentTime, float InterpolationOffset, float RTT)
{
	float RollbackTime = CurrentTime - InterpolationOffset - RTT;
	for (auto& Elem : MovementTimelines) 
	{
		if (Elem.Key == ExcludedMovable) continue;
		//UE_LOG(LogTemp, Warning, TEXT("RTT: %f"), RTT);
		//UE_LOG(LogTemp, Warning, TEXT("Int offs: %f"), InterpolationOffset);
		//UE_LOG(LogTemp, Warning, TEXT("Get snapshot time: %f"), RollbackTime);
		RepSnapshot RollbackSnapshot = Elem.Value.GetSnapshot(RollbackTime);
		Elem.Key->RollbackMovement(RollbackSnapshot);
	}
	for (auto& Elem : AnimationTimelines) 
	{
		if (Elem.Key == ExcludedMovable) continue;
		RepAnimationSnapshot RollbackSnapshot = Elem.Value.GetSnapshot(RollbackTime);
		Elem.Key->RollbackAnimation(RollbackSnapshot);
	}
}

void URepWorldTimelines::ResetWorld(IRepMovable* ExcludedMovable)
{
	for (IRepMovable* RepObject : RepObjects) {
		if (RepObject == ExcludedMovable) continue;
		RepObject->ResetRollback();
	}
}
