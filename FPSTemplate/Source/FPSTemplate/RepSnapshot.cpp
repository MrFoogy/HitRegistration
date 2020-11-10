// Fill out your copyright notice in the Description page of Project Settings.


#include "RepSnapshot.h"

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
