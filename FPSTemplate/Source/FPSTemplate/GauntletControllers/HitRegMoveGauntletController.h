// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/FPSTemplateCharacter.h"
#include "GauntletTestController.h"
#include "HitRegMoveGauntletController.generated.h"

/**
 * 
 */
UCLASS()
class FPSTEMPLATE_API UHitRegMoveGauntletController : public UGauntletTestController
{
	GENERATED_BODY()
private:
	bool Initialized = false;

	// Time to wait after game start before doing anything.
	const float SpinUpTime = 2.f;

	const float PrepareTime = 1.5f;

	// Time to run the profiler for.
	const float TestDuration = 10.f;

	const float ShutDownTime = 1.f;

	AFPSTemplateCharacter* Character;

	UFUNCTION()
	void PrepareTest();

	UFUNCTION()
	void StartTesting();

	UFUNCTION()
	void StopTesting();

protected:
	virtual void OnInit() override;
	virtual void OnTick(float DeltaTime) override;
};
