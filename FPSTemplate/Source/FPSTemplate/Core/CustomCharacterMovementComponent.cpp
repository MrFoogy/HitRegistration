// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/CustomCharacterMovementComponent.h"
#include "Runtime/Engine/Public/EngineGlobals.h"

UCustomCharacterMovementComponent::UCustomCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), RepMovementTimeline()
{
}

void UCustomCharacterMovementComponent::MoveSmooth(const FVector& InVelocity, const float DeltaSeconds, FStepDownResult* OutStepDownResult)
{
	// This function is supposed to simulate movement for replicated proxies based on the current velocity
	// By overriding this and not calling the Super function, we can disable this aspect of the extrapolation code

	// Instead, update the transform based on interpolation
	float InterpolationTime = GetWorld()->GetTimeSeconds() - RepTimeline<RepSnapshot>::InterpolationOffset;
	RepSnapshot MovementSnapshot = RepMovementTimeline.GetSnapshot(InterpolationTime);
	ApplySnapshot(MovementSnapshot);
}

void UCustomCharacterMovementComponent::SmoothClientPosition(float DeltaSeconds) 
{
	// This function is supposed to reduce each frame the mesh offset produced by extrapolation errors
	// By overriding this and not calling the Super function, we can disable this aspect of the extrapolation code
}

void UCustomCharacterMovementComponent::SmoothCorrection(const FVector& OldLocation, const FQuat& OldRotation,
	const FVector& NewLocation, const FQuat& NewRotation)
{
	// This is run after receiving a new update from the server
	// This function is supposed to snap the transform of the actor to the newly received values and apply a mesh offset
	// By overriding this and not calling the Super function, we can disable this aspect of the extrapolation code
}

void UCustomCharacterMovementComponent::OnReceiveServerUpdate(const FVector& NewLocation, 
	const FQuat& NewRotation, const FVector& NewVelocity, float ReplicationFrequency)
{

	float InterpolationTime = GetWorld()->GetTimeSeconds() - RepTimeline<RepSnapshot>::InterpolationOffset;
	RepMovementTimeline.AddSnapshotCompensating(RepSnapshot(NewLocation, NewRotation, NewVelocity), GetWorld()->GetTimeSeconds(),
		InterpolationTime, ReplicationFrequency);
}

void UCustomCharacterMovementComponent::ApplySnapshot(const RepSnapshot& Snapshot)
{
	UpdatedComponent->SetWorldLocationAndRotation(Snapshot.Position, Snapshot.Rotation, /*bSweep=*/ false);
}

FVector UCustomCharacterMovementComponent::GetRepVelocity()
{
	//UE_LOG(LogTemp, Warning, TEXT("Rep time anim: %f"), GetWorld()->GetTimeSeconds() - RepTimeline<RepSnapshot>::InterpolationOffset);
	float InterpolationTime = GetWorld()->GetTimeSeconds() - RepTimeline<RepSnapshot>::InterpolationOffset;
	return RepMovementTimeline.GetSnapshot(InterpolationTime).Velocity;
}
