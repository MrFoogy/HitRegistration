// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysXIncludes.h"
#include "PhysicsPublic.h"	
#include "Runtime/Engine/Private/PhysicsEngine/PhysXSupport.h"
#include "ShapeManagerComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSTEMPLATE_API UShapeManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UShapeManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	USkeletalMeshComponent* ShapeSourceMesh;
	TArray<physx::PxShape*> AllShapes;
	TMap<physx::PxShape*, int> ShapeIDs;

	TArray<physx::PxShape*>& GetAllShapes();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void DrawHitboxes(const FColor& Color, float LifeTime);

	void TestDisplaceHitboxes();

	/** 
	 * Calls a function for each Physics Shape on the character
	 * @param Function the function to be called
	 */
	template<typename F>
	void PerformPhysicsShapeOperation(F Function);

	void TransformAllHitboxes(FTransform Transform);

	void SavePhysicsShapeTransformsLocal(TMap<physx::PxShape*, physx::PxTransform>& OutTransforms);

	void SetPhysicsShapeTransformsLocal(TMap<physx::PxShape*, physx::PxTransform>& Transforms);

	void SavePhysicsShapeTransformsGlobal(TMap<physx::PxShape*, physx::PxTransform>& OutTransforms);

	void SetPhysicsShapeTransformsGlobal(TMap<physx::PxShape*, physx::PxTransform>& Transforms);

		
};
