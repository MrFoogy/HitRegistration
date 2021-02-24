// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhysXIncludes.h"
#include "PhysicsPublic.h"	
#include "Runtime/Engine/Private/PhysicsEngine/PhysXSupport.h"

/**
 * 
 */
class FPSTEMPLATE_API RepAnimationSnapshot
{
public:
	RepAnimationSnapshot();
	RepAnimationSnapshot(const TMap<physx::PxShape*, physx::PxTransform>& STransforms);
	RepAnimationSnapshot(const TArray<physx::PxShape*>& Shapes);
	~RepAnimationSnapshot();
	TMap<physx::PxShape*, physx::PxTransform> ShapeTransforms;
	void SetShapeTransform(physx::PxShape* Shape, physx::PxTransform Transform);
	bool HasAddedAllTransforms() const;

	const TMap<physx::PxShape*, physx::PxTransform>& GetShapeTransforms() const;

	static RepAnimationSnapshot Interpolate(const RepAnimationSnapshot& Start, const RepAnimationSnapshot& End, float Alpha);

	static float GetAverageAngleDiscrepancy(const RepAnimationSnapshot& Snapshot1, const RepAnimationSnapshot& Snapshot2);
	static float GetAveragePositionDiscrepancy(const RepAnimationSnapshot& Snapshot1, const RepAnimationSnapshot& Snapshot2);

protected:
	int AddedTransforms;
};
