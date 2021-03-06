// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/FPSTemplateCharacter.h"
#include "GauntletTestController.h"
#include "HitRegGauntletController.generated.h"

/**
 * 
 */
UCLASS()
class FPSTEMPLATE_API UHitRegGauntletController : public UGauntletTestController
{
	GENERATED_BODY()
private:
	bool Initialized = false;

	// Time to run the profiler for.
	const float TestDuration = 22.5f;

	bool UseRollback;

	UFUNCTION()
	void StartTesting();

	UFUNCTION()
	void StopTesting();

protected:
	virtual void OnInit() override;
	virtual void OnTick(float DeltaTime) override;
};
