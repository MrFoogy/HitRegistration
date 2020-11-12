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

RepAnimationSnapshot RepAnimationSnapshot::Interpolate(const RepAnimationSnapshot& Start, const RepAnimationSnapshot& End, float Alpha)
{
	RepAnimationSnapshot InterpolatedSnapshot;
	for (auto& StartElem : Start.ShapeTransforms) {
		physx::PxShape* Shape = StartElem.Key;
		const physx::PxTransform& StartTransform = StartElem.Value;
		const physx::PxTransform& EndTransform = End.ShapeTransforms[Shape];
		physx::PxVec3 PxPosition = ((StartTransform.p * (1.0f - Alpha)) + (EndTransform.p * Alpha));
		physx::PxQuat PxRotation = U2PQuat(FQuat::FastLerp(P2UQuat(StartTransform.q), P2UQuat(EndTransform.q), Alpha));
		InterpolatedSnapshot.SetShapeTransform(Shape, physx::PxTransform(PxPosition, PxRotation));
	}

	return InterpolatedSnapshot;
}
