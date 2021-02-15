// Fill out your copyright notice in the Description page of Project Settings.

#include "ShapeManagerComponent.h"
#include "CollisionTestActor.h"

ACollisionTestActor::ACollisionTestActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TestMesh"));

	ShapeManager = CreateDefaultSubobject<UShapeManagerComponent>(TEXT("ShapeManager"));
	ShapeManager->ShapeSourceMesh = TestMesh;
}

/*
// Sets default values
ACollisionTestActor::ACollisionTestActor()
{

}
*/

// Called when the game starts or when spawned
void ACollisionTestActor::BeginPlay()
{
	Super::BeginPlay();
	
	FTimerHandle dummy;
	float TestWaitTime = 3.0f;
	GetWorld()->GetTimerManager().SetTimer(dummy, this, &ACollisionTestActor::PerformTriggerTest, TestWaitTime, false);
}

// Called every frame
void ACollisionTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//SetActorLocation(GetActorLocation() + FVector(0.f, DeltaTime * 100.f, 0.f));;

	ShapeManager->DrawHitboxes(FColor::Green, 0.3f);
}

void ACollisionTestActor::PerformTriggerTest()
{
	// This move should trigger a collision if left permanent
	ShapeManager->TransformAllHitboxes(FTransform(FVector(0.0f, 200.0f, 0.0f)));

	// But by moving the shapes out of the trigger right after, verify that no collision is triggered
	ShapeManager->TransformAllHitboxes(FTransform(FVector(100.0f, 0.0f, 0.0f)));
}

