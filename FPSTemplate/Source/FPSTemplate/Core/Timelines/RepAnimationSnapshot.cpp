// Fill out your copyright notice in the Description page of Project Settings.


#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "Core/Timelines/RepAnimationSnapshot.h"

RepAnimationSnapshot::RepAnimationSnapshot()
{
	AddedTransforms = 0;
}

RepAnimationSnapshot::RepAnimationSnapshot(const TMap<physx::PxShape*, physx::PxTransform>& STransforms)
{
	ShapeTransforms = STransforms;
	AddedTransforms = 0;
}
	
RepAnimationSnapshot::RepAnimationSnapshot(const TArray<physx::PxShape*>& Shapes)
{
	ShapeTransforms = TMap<PxShape*, PxTransform>();
	for (PxShape* Shape : Shapes) {
		ShapeTransforms.Add(Shape, PxTransform());
	}
	AddedTransforms = 0;
}

RepAnimationSnapshot::~RepAnimationSnapshot()
{
}

void RepAnimationSnapshot::SetShapeTransform(physx::PxShape* Shape, physx::PxTransform Transform) 
{
	ShapeTransforms.Add(Shape, Transform);
	AddedTransforms++;
}

const TMap<physx::PxShape*, physx::PxTransform>& RepAnimationSnapshot::GetShapeTransforms() const
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
		physx::PxQuat PxRotation = U2PQuat(FQuat::FastLerp(P2UQuat(StartTransform.q), P2UQuat(EndTransform.q), Alpha).GetNormalized());
		InterpolatedSnapshot.SetShapeTransform(Shape, physx::PxTransform(PxPosition, PxRotation));
	}

	return InterpolatedSnapshot;
}

float RepAnimationSnapshot::GetAverageAngleDiscrepancy(const RepAnimationSnapshot& Snapshot1, const RepAnimationSnapshot& Snapshot2)
{
	float TotalAngleDiff = 0.0f;
	for (auto KV : Snapshot1.GetShapeTransforms()) {
		physx::PxShape* Shape = KV.Key;
		physx::PxTransform Transform1 = Snapshot1.GetShapeTransforms()[Shape];
		physx::PxTransform Transform2 = Snapshot2.GetShapeTransforms()[Shape];
		float AngleDiff = Transform1.q.getAngle(Transform2.q);
		AngleDiff = FMath::Min(AngleDiff, 2.0f * UKismetMathLibrary::GetPI() - AngleDiff);
		TotalAngleDiff += AngleDiff;
		float DistDiff = (Transform1.p - Transform2.p).magnitude();
	}
	return TotalAngleDiff / Snapshot1.GetShapeTransforms().Num();
}

float RepAnimationSnapshot::GetAveragePositionDiscrepancy(const RepAnimationSnapshot& Snapshot1, const RepAnimationSnapshot& Snapshot2)
{
	float TotalDistDiff = 0.0f;
	for (auto KV : Snapshot1.GetShapeTransforms()) {
		physx::PxShape* Shape = KV.Key;
		physx::PxTransform Transform1 = Snapshot1.GetShapeTransforms()[Shape];
		physx::PxTransform Transform2 = Snapshot2.GetShapeTransforms()[Shape];
		float AngleDiff = Transform1.q.getAngle(Transform2.q);
		float DistDiff = (Transform1.p - Transform2.p).magnitude();
		TotalDistDiff += DistDiff;
	}
	return TotalDistDiff / Snapshot1.GetShapeTransforms().Num();
}

bool RepAnimationSnapshot::HasAddedAllTransforms() const {
	// Assumes the Map is initialized in the beginning to contain all keys
	return AddedTransforms == ShapeTransforms.Num();
}
