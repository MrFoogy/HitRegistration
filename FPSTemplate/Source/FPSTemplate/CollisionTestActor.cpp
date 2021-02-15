// Fill out your copyright notice in the Description page of Project Settings.


#include "CollisionTestActor.h"

ACollisionTestActor::ACollisionTestActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TestMesh"));
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
	
}

// Called every frame
void ACollisionTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetActorLocation(GetActorLocation() + FVector(0.f, DeltaTime * 100.f, 0.f));;
}

