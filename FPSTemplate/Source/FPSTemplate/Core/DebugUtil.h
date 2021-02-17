// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PhysXIncludes.h"
#include "PhysicsPublic.h"	
#include "Runtime/Engine/Private/PhysicsEngine/PhysXSupport.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "CoreMinimal.h"

/**
 * 
 */
class FPSTEMPLATE_API DebugUtil
{
public:
	DebugUtil();
	~DebugUtil();

	static void DrawPxShape(UWorld* World, physx::PxRigidActor* PxActor, physx::PxShape* Shape, FColor Color, float LifeTime);
	static void DrawPxShape(UWorld* World, physx::PxShape* Shape, FVector Position, FQuat Rotation, FColor Color, float LifeTime);
};
