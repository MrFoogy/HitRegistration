// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCharacterMovementComponent.h"
#include "Runtime/Engine/Public/EngineGlobals.h"

UCustomCharacterMovementComponent::UCustomCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), RepMovementTimeline()
{
}

void UCustomCharacterMovementComponent::MoveSmooth(const FVector& InVelocity, const float DeltaSeconds, FStepDownResult* OutStepDownResult)
{
	//Super::MoveSmooth(InVelocity, DeltaSeconds, OutStepDownResult);
	//UE_LOG(LogTemp, Warning, TEXT("Rep time move: %f"), GetWorld()->GetTimeSeconds() - RepTimeline<RepSnapshot>::InterpolationOffset);
	float InterpolationTime = GetWorld()->GetTimeSeconds() - RepTimeline<RepSnapshot>::InterpolationOffset;
	RepSnapshot MovementSnapshot = RepMovementTimeline.GetSnapshot(InterpolationTime);
	ApplySnapshot(MovementSnapshot);
}

void UCustomCharacterMovementComponent::SmoothClientPosition(float DeltaSeconds) 
{
	//Super::SmoothClientPosition(DeltaSeconds);
}

void UCustomCharacterMovementComponent::SmoothCorrection(const FVector& OldLocation, const FQuat& OldRotation,
	const FVector& NewLocation, const FQuat& NewRotation)
{
	//UpdatedComponent->SetWorldLocationAndRotation(NewLocation, NewRotation, /*bSweep=*/ false);

	// This is run after receiving a new update from the server
}

void UCustomCharacterMovementComponent::OnReceiveServerUpdate(const FVector& NewLocation, 
	const FQuat& NewRotation, const FVector& NewVelocity, float ReplicationFrequency)
{
	//UpdatedComponent->SetWorldLocationAndRotation(NewLocation, NewRotation, /*bSweep=*/ false);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Server update at: %f"), GetWorld()->GetTimeSeconds()));

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
