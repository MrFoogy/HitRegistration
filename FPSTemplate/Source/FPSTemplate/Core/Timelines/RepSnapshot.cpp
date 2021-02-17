// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Timelines/RepSnapshot.h"

RepSnapshot::RepSnapshot() : 
	Position(ForceInitToZero),
	Rotation(ForceInitToZero),
	Velocity(ForceInitToZero)
{

}

RepSnapshot::RepSnapshot(FVector Pos, FQuat Rot, FVector Vel)
	: Position{ Pos }, Rotation{ Rot }, Velocity{ Vel }
{

}

RepSnapshot::~RepSnapshot()
{

}

RepSnapshot RepSnapshot::Interpolate(const RepSnapshot& Start, const RepSnapshot& End, float Alpha) {
	RepSnapshot InterpolatedSnapshot;
	InterpolatedSnapshot.Position = FMath::LerpStable(Start.Position, End.Position, Alpha);
	InterpolatedSnapshot.Rotation = FQuat::Slerp(Start.Rotation, End.Rotation, Alpha);
	InterpolatedSnapshot.Velocity = FMath::LerpStable(Start.Velocity, End.Velocity, Alpha);

	return InterpolatedSnapshot;
}
