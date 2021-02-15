// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugUtil.h"
#include "ShapeManagerComponent.h"

// Sets default values for this component's properties
UShapeManagerComponent::UShapeManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UShapeManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Store the shapes
	auto ShapeFunction = [&ShapesArray = AllShapes, &IDMap = ShapeIDs](physx::PxRigidActor* PxActor, physx::PxShape* PxShape, int ID)
	{
		physx::PxTransform ActorGlobalPose = PxActor->getGlobalPose();
		physx::PxTransform ShapeGlobalPose = ActorGlobalPose * PxShape->getLocalPose();
		if (ShapesArray.Num() < ID + 1) {
			ShapesArray.SetNum(ID + 1);
		}
		ShapesArray[ID] = PxShape;
		IDMap.Add(PxShape, ID);
	};
	PerformPhysicsShapeOperation(ShapeFunction);
}


// Called every frame
void UShapeManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

template<typename F>
void UShapeManagerComponent::PerformPhysicsShapeOperation(F Function)
{
	FBodyInstance* PhysicsBody = ShapeSourceMesh->GetBodyInstance();
	PxScene* Scene = GetWorld()->GetPhysicsScene()->GetPxScene();
	TArray<FPhysicsShapeHandle> collisionShapes;
	Scene->lockRead();
	// First gain access to at least one of the physics shapes
	int32 numSyncShapes = PhysicsBody->GetAllShapes_AssumesLocked(collisionShapes);
	Scene->unlockRead();

	// Use one of the shapes to find all PxActors
	int NumActors = collisionShapes[0].Shape->getActor()->getAggregate()->getNbActors();
	physx::PxActor** PxActors = new physx::PxActor*[NumActors];
	physx::PxShape* PxShapes[10];
	int FoundActors = collisionShapes[0].Shape->getActor()->getAggregate()->getActors(PxActors, NumActors);

	// For each PxActor, find all its shapes and execute the input function
	int Index = 0;
	for (int i = 0; i < FoundActors; i++) {
		physx::PxRigidActor* RigidActor = (physx::PxRigidActor*) PxActors[i];
		if (RigidActor == NULL) continue;
		int FoundShapes = RigidActor->getShapes(PxShapes, 10);
		for (int j = 0; j < FoundShapes; j++) {
			Function(RigidActor, PxShapes[j], Index);
			Index++;
		}
	}
	delete[] PxActors;
}

void UShapeManagerComponent::DrawHitboxes(const FColor& Color, float LifeTime) 
{
	auto World = GetWorld();
	auto ShapeFunction = [Color, LifeTime, World](physx::PxRigidActor* PxActor, physx::PxShape* PxShape, int ID)
	{
		DebugUtil::DrawPxShape(World, PxActor, PxShape, Color, LifeTime);
	};
	PerformPhysicsShapeOperation(ShapeFunction);
}

void UShapeManagerComponent::TransformAllHitboxes(FTransform Transform)
{
	auto ShapeFunction = [Transform](physx::PxRigidActor* PxActor, physx::PxShape* PxShape, int ID)
	{
		physx::PxTransform ActorGlobalPose = PxActor->getGlobalPose();
		physx::PxTransform ShapeGlobalPose = ActorGlobalPose * PxShape->getLocalPose();
		physx::PxTransform DesiredGlobalTransform = U2PTransform(Transform) * ShapeGlobalPose;
		PxShape->setLocalPose(physx::PxTransform(ActorGlobalPose.getInverse() * DesiredGlobalTransform));
	};
	PerformPhysicsShapeOperation(ShapeFunction);
}

void UShapeManagerComponent::TestDisplaceHitboxes()
{
	auto ShapeFunction = [](physx::PxRigidActor* PxActor, physx::PxShape* PxShape, int ID)
	{
		physx::PxTransform DesiredGlobalTransform(physx::PxVec3(0.0f, 0.0f, 200.0f), physx::PxQuat(physx::PxIdentity));
		physx::PxTransform ActorGlobalPose = PxActor->getGlobalPose();
		PxShape->setLocalPose(physx::PxTransform(ActorGlobalPose.getInverse() * DesiredGlobalTransform));
	};
	PerformPhysicsShapeOperation(ShapeFunction);
}

void UShapeManagerComponent::SavePhysicsShapeTransformsLocal(TMap<physx::PxShape*, physx::PxTransform>& OutTransforms)
{
	auto ShapeFunction = [&OutTransforms](physx::PxRigidActor* PxActor, physx::PxShape* PxShape, int ID)
	{
		OutTransforms.Add(PxShape, PxShape->getLocalPose());
	};
	PerformPhysicsShapeOperation(ShapeFunction);
}

void UShapeManagerComponent::SetPhysicsShapeTransformsLocal(TMap<physx::PxShape*, physx::PxTransform>& Transforms)
{
	auto ShapeFunction = [&Transforms](physx::PxRigidActor* PxActor, physx::PxShape* PxShape, int ID)
	{
		PxShape->setLocalPose(Transforms[PxShape]);
	};
	PerformPhysicsShapeOperation(ShapeFunction);
}

void UShapeManagerComponent::SavePhysicsShapeTransformsGlobal(TMap<physx::PxShape*, physx::PxTransform>& OutTransforms)
{
	auto ShapeFunction = [&OutTransforms](physx::PxRigidActor* PxActor, physx::PxShape* PxShape, int ID)
	{
		physx::PxTransform ActorGlobalPose = PxActor->getGlobalPose();
		physx::PxTransform ShapeGlobalPose = ActorGlobalPose * PxShape->getLocalPose();
		OutTransforms.Add(PxShape, ShapeGlobalPose);
	};
	PerformPhysicsShapeOperation(ShapeFunction);
}

void UShapeManagerComponent::SetPhysicsShapeTransformsGlobal(TMap<physx::PxShape*, physx::PxTransform>& Transforms)
{
	auto ShapeFunction = [&Transforms](physx::PxRigidActor* PxActor, physx::PxShape* PxShape, int ID)
	{
		physx::PxTransform ActorGlobalPose = PxActor->getGlobalPose();
		PxShape->setLocalPose(ActorGlobalPose.getInverse() * Transforms[PxShape]);
	};
	PerformPhysicsShapeOperation(ShapeFunction);
}

TArray<physx::PxShape*>& UShapeManagerComponent::GetAllShapes()
{
	return AllShapes;
}

