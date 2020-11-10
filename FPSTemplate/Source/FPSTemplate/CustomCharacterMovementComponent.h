// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RepMovementTimeline.h"
#include "RepSnapshot.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class FPSTEMPLATE_API UCustomCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

public:
	void MoveSmooth(const FVector& InVelocity, const float DeltaSeconds, FStepDownResult* OutStepDownResult) override;
	void SmoothCorrection(const FVector& OldLocation, const FQuat& OldRotation,
		const FVector& NewLocation, const FQuat& NewRotation) override;

	void OnReceiveServerUpdate(const FVector& NewLocation, const FQuat& NewRotation, const FVector& NewVelocity);
	virtual void ApplySnapshot(const RepSnapshot& Spanshot);

protected:
	void SmoothClientPosition(float DeltaSeconds) override;

	RepMovementTimeline ReplicationTimeline;
};
