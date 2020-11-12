// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class FPSTEMPLATE_API RepViewRotationSnapshot
{
public:
	RepViewRotationSnapshot();
	RepViewRotationSnapshot(FRotator ViewRotation);
	~RepViewRotationSnapshot();

	FRotator ViewRotation;
	static RepViewRotationSnapshot Interpolate(const RepViewRotationSnapshot& Start, const RepViewRotationSnapshot& End, float Alpha);
};
