// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CollisionTestActor.generated.h"

class UShapeManagerComponent;

UCLASS()
class FPSTEMPLATE_API ACollisionTestActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* TestMesh;

	UPROPERTY(VisibleAnywhere, Category = "HitReg")
	class UShapeManagerComponent* ShapeManager;
	
public:	
	ACollisionTestActor(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	void PerformTriggerTest();
};
