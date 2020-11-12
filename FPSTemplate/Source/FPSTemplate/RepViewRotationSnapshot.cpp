// Fill out your copyright notice in the Description page of Project Settings.


#include "RepViewRotationSnapshot.h"

RepViewRotationSnapshot::RepViewRotationSnapshot() : ViewRotation { ForceInitToZero }
{
}

RepViewRotationSnapshot::~RepViewRotationSnapshot()
{
}

RepViewRotationSnapshot::RepViewRotationSnapshot(FRotator Rotation) : ViewRotation{Rotation}
{
}

RepViewRotationSnapshot RepViewRotationSnapshot::Interpolate(const RepViewRotationSnapshot& Start, const RepViewRotationSnapshot& End, float Alpha)
{
	FQuat StartQuat = Start.ViewRotation.Quaternion();
	FQuat EndQuat = End.ViewRotation.Quaternion();
	return RepViewRotationSnapshot(FQuat::Slerp(StartQuat, EndQuat, Alpha).Rotator());
}
