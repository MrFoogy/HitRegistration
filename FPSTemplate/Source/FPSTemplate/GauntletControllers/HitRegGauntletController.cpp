// Fill out your copyright notice in the Description page of Project Settings.

#include "HitRegGauntletController.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "TimerManager.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Async/Async.h"

void UHitRegGauntletController::OnInit()
{
    UE_LOG(LogGauntlet, Display, TEXT("HitRegGauntletController started"));

	UseRollback = false;

	FString UseRollbackParam;
	if (FParse::Value(FCommandLine::Get(), TEXT("Rollback"), UseRollbackParam)) {
		UseRollbackParam = UseRollbackParam.Replace(TEXT("="), TEXT("")).Replace(TEXT("\""), TEXT(""));
	}
    UE_LOG(LogGauntlet, Display, TEXT("Use rollback: %s"), *UseRollbackParam);
	if (UseRollbackParam == "True" || UseRollbackParam == "true") {
		UseRollback = true;
	}
}

void UHitRegGauntletController::StartTesting()
{
	// This is not run currently
}

void UHitRegGauntletController::OnTick(float DeltaTime)
{
    if (!Initialized && GetWorld() != NULL) {
		FTimerHandle dummy;
		GetWorld()->GetTimerManager().SetTimer(dummy, this, &UHitRegGauntletController::StopTesting, TestDuration, false);

		UE_LOG(LogGauntlet, Display, TEXT("Are we here?"));

		AFPSTemplateGameMode* GameMode = (AFPSTemplateGameMode*)GetWorld()->GetAuthGameMode();
		if (GameMode != NULL) {
			GameMode->ShouldUseRollback = UseRollback;
			if (UseRollback) {
				UE_LOG(LogGauntlet, Display, TEXT("Should use rollback? YES"));
			}
			else {
				UE_LOG(LogGauntlet, Display, TEXT("Should use rollback? NO"));
			}
		}

		/*
		FPacketSimulationSettings CustomSettings;
		CustomSettings.PktIncomingLagMin = 300;
		CustomSettings.PktIncomingLagMax = 300;
		GetWorld()->GetNetDriver()->SetPacketSimulationSettings(CustomSettings);
		*/

		Initialized = true;
	}
}

void UHitRegGauntletController::StopTesting()
{
	EndTest(0);
}
