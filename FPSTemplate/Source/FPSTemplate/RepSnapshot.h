// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Misc/CoreMiscDefines.h"
#include "CoreMinimal.h"

/**
 * 
 */
class FPSTEMPLATE_API RepSnapshot
{
public:
	RepSnapshot();
	RepSnapshot(FVector Pos, FQuat Rot, FVector Vel);
	~RepSnapshot();

	FVector Position;
	FQuat Rotation;
	FVector Velocity;

	static RepSnapshot Interpolate(const RepSnapshot& Start, const RepSnapshot& End, float Alpha);
};
