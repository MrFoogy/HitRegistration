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
	~RepAnimationSnapshot();
	TMap<physx::PxShape*, physx::PxTransform> ShapeTransforms;
	void SetShapeTransform(physx::PxShape* Shape, physx::PxTransform Transform);

	TMap<physx::PxShape*, physx::PxTransform>& GetShapeTransforms();
};
