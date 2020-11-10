// Fill out your copyright notice in the Description page of Project Settings.


#include "RepAnimationSnapshot.h"

RepAnimationSnapshot::RepAnimationSnapshot()
{
}

RepAnimationSnapshot::RepAnimationSnapshot(const TMap<physx::PxShape*, physx::PxTransform>& STransforms)
{
	ShapeTransforms = STransforms;
}

RepAnimationSnapshot::~RepAnimationSnapshot()
{
}

void RepAnimationSnapshot::SetShapeTransform(physx::PxShape* Shape, physx::PxTransform Transform) 
{
	ShapeTransforms.Add(Shape, Transform);
}

TMap<physx::PxShape*, physx::PxTransform>& RepAnimationSnapshot::GetShapeTransforms()
{
	return ShapeTransforms;
}
