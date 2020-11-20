// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugUtil.h"

DebugUtil::DebugUtil()
{
}

DebugUtil::~DebugUtil()
{
}
	
void DebugUtil::DrawPxShape(UWorld* World, physx::PxRigidActor* PxActor, physx::PxShape* Shape, FColor Color, float LifeTime)
{
	physx::PxTransform ActorGlobalPose = PxActor->getGlobalPose();
	physx::PxTransform ShapeGlobalPose = ActorGlobalPose * Shape->getLocalPose();
	DrawPxShape(World, Shape, P2UVector(ShapeGlobalPose.p), P2UQuat(ShapeGlobalPose.q), Color, LifeTime);
}

void DebugUtil::DrawPxShape(UWorld* World, physx::PxShape* Shape, FVector Position, FQuat Rotation, FColor Color, float LifeTime)
{
	physx::PxGeometryType::Enum GeometryType = Shape->getGeometryType();
	if (GeometryType == physx::PxGeometryType::Enum::eCAPSULE) {
		physx::PxCapsuleGeometry CapsuleGeometry;
		Shape->getCapsuleGeometry(CapsuleGeometry);

		DrawDebugCapsule(World, Position, CapsuleGeometry.halfHeight * 2.0f, CapsuleGeometry.radius,
			Rotation * FQuat(FVector::RightVector, UKismetMathLibrary::GetPI() / 2.0f), Color, false, LifeTime);
	}
	if (GeometryType == physx::PxGeometryType::Enum::eBOX) {
		physx::PxBoxGeometry BoxGeometry;
		Shape->getBoxGeometry(BoxGeometry);

		DrawDebugBox(World, Position, P2UVector(BoxGeometry.halfExtents), Rotation,
			Color, false, LifeTime);
	}
}
